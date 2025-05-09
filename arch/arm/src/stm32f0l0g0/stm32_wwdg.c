/****************************************************************************
 * arch/arm/src/stm32f0l0g0/stm32_wwdg.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/arch.h>

#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/timers/watchdog.h>
#include <arch/board/board.h>

#include "arm_internal.h"
#include "hardware/stm32_dbgmcu.h"
#include "stm32_wdg.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Clocking *****************************************************************/

/* The minimum frequency of the WWDG clock is:
 *
 *  Fmin = PCLK1 / 4096 / 8
 *
 * So the maximum delay (in milliseconds) is then:
 *
 *   1000 * (WWDG_CR_T_MAX+1) / Fmin
 *
 * For example, if PCLK1 = 42MHz, then the maximum delay is:
 *
 *   Fmin = 1281.74
 *   1000 * 64 / Fmin = 49.93 msec
 */

#define WWDG_FMIN       (STM32_PCLK1_FREQUENCY / 4096 / 8)
#define WWDG_MAXTIMEOUT (1000 * (WWDG_CR_T_MAX+1) / WWDG_FMIN)

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure provides the private representation of the "lower-half"
 * driver state structure.  This structure must be cast-compatible with the
 * well-known watchdog_lowerhalf_s structure.
 */

struct stm32_lowerhalf_s
{
  const struct watchdog_ops_s *ops; /* Lower half operations */
  xcpt_t   handler;                 /* Current EWI interrupt handler */
  uint32_t timeout;                 /* The actual timeout value */
  uint32_t fwwdg;                   /* WWDG clock frequency */
  bool     started;                 /* The timer has been started */
  uint8_t  reload;                  /* The 7-bit reload field reset value */
  uint8_t  window;                  /* The 7-bit window (W) field value */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/* Register operations ******************************************************/

static void stm32_setwindow(struct stm32_lowerhalf_s *priv, uint8_t window);

/* Interrupt handling *******************************************************/

static int stm32_interrupt(int irq, void *context, void *arg);

/* "Lower half" driver methods **********************************************/

static int stm32_start(struct watchdog_lowerhalf_s *lower);
static int stm32_stop(struct watchdog_lowerhalf_s *lower);
static int stm32_keepalive(struct watchdog_lowerhalf_s *lower);
static int stm32_getstatus(struct watchdog_lowerhalf_s *lower,
                           struct watchdog_status_s *status);
static int stm32_settimeout(struct watchdog_lowerhalf_s *lower,
                            uint32_t timeout);
static xcpt_t stm32_capture(struct watchdog_lowerhalf_s *lower,
                            xcpt_t handler);
static int  stm32_ioctl(struct watchdog_lowerhalf_s *lower, int cmd,
                        unsigned long arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* "Lower half" driver methods */

static const struct watchdog_ops_s g_wdgops =
{
  .start      = stm32_start,
  .stop       = stm32_stop,
  .keepalive  = stm32_keepalive,
  .getstatus  = stm32_getstatus,
  .settimeout = stm32_settimeout,
  .capture    = stm32_capture,
  .ioctl      = stm32_ioctl,
};

/* "Lower half" driver state */

static struct stm32_lowerhalf_s g_wdgdev;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_setwindow
 *
 * Description:
 *   Set the CFR window value. The window value is compared to the down-
 *   counter when the counter is updated.  The WWDG counter should be updated
 *   only when the counter is below this window value (and greater than 64)
 *   otherwise a reset will be generated
 *
 ****************************************************************************/

static void stm32_setwindow(struct stm32_lowerhalf_s *priv, uint8_t window)
{
  uint16_t regval;

  /* Set W[6:0] bits according to selected window value */

  regval = getreg32(STM32_WWDG_CFR);
  regval &= ~WWDG_CFR_W_MASK;
  regval |= window << WWDG_CFR_W_SHIFT;
  putreg32(regval, STM32_WWDG_CFR);

  /* Remember the window setting */

  priv->window = window;
}

/****************************************************************************
 * Name: stm32_interrupt
 *
 * Description:
 *   WWDG early warning interrupt
 *
 * Input Parameters:
 *   Usual interrupt handler arguments.
 *
 * Returned Value:
 *   Always returns OK.
 *
 ****************************************************************************/

static int stm32_interrupt(int irq, void *context, void *arg)
{
  struct stm32_lowerhalf_s *priv = &g_wdgdev;
  uint16_t regval;

  /* Check if the EWI interrupt is really pending */

  regval = getreg32(STM32_WWDG_SR);
  if ((regval & WWDG_SR_EWIF) != 0)
    {
      /* Is there a registered handler? */

      if (priv->handler)
        {
          /* Yes... NOTE:  This interrupt service routine (ISR) must reload
           * the WWDG counter to prevent the reset.  Otherwise, we will reset
           * upon return.
           */

          priv->handler(irq, context, arg);
        }

      /* The EWI interrupt is cleared by writing '0' to the EWIF bit in the
       * WWDG_SR register.
       */

      regval &= ~WWDG_SR_EWIF;
      putreg32(regval, STM32_WWDG_SR);
    }

  return OK;
}

/****************************************************************************
 * Name: stm32_start
 *
 * Description:
 *   Start the watchdog timer, resetting the time to the current timeout,
 *
 * Input Parameters:
 *   lower - A pointer the publicly visible representation of the "lower-
 *           half" driver state structure.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int stm32_start(struct watchdog_lowerhalf_s *lower)
{
  struct stm32_lowerhalf_s *priv = (struct stm32_lowerhalf_s *)lower;

  wdinfo("Entry\n");
  DEBUGASSERT(priv);

  /* The watchdog is always disabled after a reset. It is enabled by setting
   * the WDGA bit in the WWDG_CR register, then it cannot be disabled again
   * except by a reset.
   */

  putreg32(WWDG_CR_WDGA | WWDG_CR_T_RESET | priv->reload, STM32_WWDG_CR);
  priv->started = true;
  return OK;
}

/****************************************************************************
 * Name: stm32_stop
 *
 * Description:
 *   Stop the watchdog timer
 *
 * Input Parameters:
 *   lower - A pointer the publicly visible representation of the "lower-
 *           half" driver state structure.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int stm32_stop(struct watchdog_lowerhalf_s *lower)
{
  /* The watchdog is always disabled after a reset. It is enabled by setting
   * the WDGA bit in the WWDG_CR register, then it cannot be disabled again
   * except by a reset.
   */

  wdinfo("Entry\n");
  return -ENOSYS;
}

/****************************************************************************
 * Name: stm32_keepalive
 *
 * Description:
 *   Reset the watchdog timer to the current timeout value, prevent any
 *   imminent watchdog timeouts.  This is sometimes referred as "pinging"
 *   the watchdog timer or "petting the dog".
 *
 *   The application program must write in the WWDG_CR register at regular
 *   intervals during normal operation to prevent an MCU reset. This
 *   operation must occur only when the counter value is lower than the
 *   window register value. The value to be stored in the WWDG_CR register
 *   must be between 0xff and 0xC0:
 *
 * Input Parameters:
 *   lower - A pointer the publicly visible representation of the "lower-
 *           half" driver state structure.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int stm32_keepalive(struct watchdog_lowerhalf_s *lower)
{
  struct stm32_lowerhalf_s *priv = (struct stm32_lowerhalf_s *)lower;

  wdinfo("Entry\n");
  DEBUGASSERT(priv);

  /* Write to T[6:0] bits to configure the counter value, no need to do
   * a read-modify-write; writing a 0 to WDGA bit does nothing.
   */

  putreg32((WWDG_CR_T_RESET | priv->reload), STM32_WWDG_CR);
  return OK;
}

/****************************************************************************
 * Name: stm32_getstatus
 *
 * Description:
 *   Get the current watchdog timer status
 *
 * Input Parameters:
 *   lower  - A pointer the publicly visible representation of the "lower-
 *            half" driver state structure.
 *   status - The location to return the watchdog status information.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int stm32_getstatus(struct watchdog_lowerhalf_s *lower,
                           struct watchdog_status_s *status)
{
  struct stm32_lowerhalf_s *priv = (struct stm32_lowerhalf_s *)lower;
  uint32_t elapsed;
  uint16_t reload;

  wdinfo("Entry\n");
  DEBUGASSERT(priv);

  /* Return the status bit */

  status->flags = WDFLAGS_RESET;
  if (priv->started)
    {
      status->flags |= WDFLAGS_ACTIVE;
    }

  if (priv->handler)
    {
      status->flags |= WDFLAGS_CAPTURE;
    }

  /* Return the actual timeout is milliseconds */

  status->timeout = priv->timeout;

  /* Get the time remaining until the watchdog expires (in milliseconds) */

  reload = (getreg32(STM32_WWDG_CR) >> WWDG_CR_T_SHIFT) & 0x7f;
  elapsed = priv->reload - reload;
  status->timeleft = (priv->timeout * elapsed) / (priv->reload + 1);

  wdinfo("Status     :\n");
  wdinfo("  flags    : %08x\n", (unsigned)status->flags);
  wdinfo("  timeout  : %u\n", (unsigned)status->timeout);
  wdinfo("  timeleft : %u\n", (unsigned)status->flags);
  return OK;
}

/****************************************************************************
 * Name: stm32_settimeout
 *
 * Description:
 *   Set a new timeout value (and reset the watchdog timer)
 *
 * Input Parameters:
 *   lower   - A pointer the publicly visible representation of the
 *             "lower-half" driver state structure.
 *   timeout - The new timeout value in milliseconds.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int stm32_settimeout(struct watchdog_lowerhalf_s *lower,
                            uint32_t timeout)
{
  struct stm32_lowerhalf_s *priv = (struct stm32_lowerhalf_s *)lower;
  uint32_t fwwdg;
  uint32_t reload;
  uint16_t regval;
  int wdgtb;

  DEBUGASSERT(priv);
  wdinfo("Entry: timeout=%u\n", (unsigned)timeout);

  /* Can this timeout be represented? */

  if (timeout < 1 || timeout > WWDG_MAXTIMEOUT)
    {
      wderr("ERROR: Cannot represent timeout=%u > %lu\n",
            (unsigned)timeout, WWDG_MAXTIMEOUT);
      return -ERANGE;
    }

  /* Determine prescaler value.
   *
   * Fwwdg = PCLK1/4096/prescaler.
   *
   * Where
   *  Fwwwdg is the frequency of the WWDG clock
   *  wdgtb is one of {1, 2, 4, or 8}
   */

  /* Select the smallest prescaler that will result in a reload field value
   * that is less than the maximum.
   */

  for (wdgtb = 0; ; wdgtb++)
    {
      /* WDGTB = 0 -> Divider = 1  = 1 << 0
       * WDGTB = 1 -> Divider = 2  = 1 << 1
       * WDGTB = 2 -> Divider = 4  = 1 << 2
       * WDGTB = 3 -> Divider = 8  = 1 << 3
       */

      /* Get the WWDG counter frequency in Hz. */

      fwwdg = (STM32_PCLK1_FREQUENCY / 4096) >> wdgtb;

      /* The formula to calculate the timeout value is given by:
       *
       * timeout =  1000 * (reload + 1) / Fwwdg, OR
       * reload = timeout * Fwwdg / 1000 - 1
       *
       * Where
       *  timeout is the desired timeout in milliseconds
       *  reload is the contents of T{5:0]
       *  Fwwdg is the frequency of the WWDG clock
       */

       reload = timeout * fwwdg / 1000 - 1;

      /* If this reload valid is less than the maximum or we are not ready
       * at the prescaler value, then break out of the loop to use these
       * settings.
       */

#if 0
      wdinfo("wdgtb=%d fwwdg=%d reload=%d timeout=%d\n",
             wdgtb, fwwdg, reload,  1000 * (reload + 1) / fwwdg);
#endif
      if (reload <= WWDG_CR_T_MAX || wdgtb == 3)
        {
          /* Note that we explicitly break out of the loop rather than using
           * the 'for' loop termination logic because we do not want the
           * value of wdgtb to be incremented.
           */

          break;
        }
    }

  /* Make sure that the final reload value is within range */

  if (reload > WWDG_CR_T_MAX)
    {
      reload = WWDG_CR_T_MAX;
    }

  /* Calculate and save the actual timeout value in milliseconds:
   *
   * timeout =  1000 * (reload + 1) / Fwwdg
   */

  priv->timeout = 1000 * (reload + 1) / fwwdg;

  /* Remember the selected values */

  priv->fwwdg  = fwwdg;
  priv->reload = reload;

  wdinfo("wdgtb=%d fwwdg=%u reload=%u timeout=%u\n",
         wdgtb, (unsigned)fwwdg, (unsigned)reload, (unsigned)priv->timeout);

  /* Set WDGTB[1:0] bits according to calculated value */

  regval = getreg32(STM32_WWDG_CFR);
  regval &= ~WWDG_CFR_WDGTB_MASK;
  regval |= (uint16_t)wdgtb << WWDG_CFR_WDGTB_SHIFT;
  putreg32(regval, STM32_WWDG_CFR);

  /* Reset the 7-bit window value to the maximum value.. essentially
   * disabling the lower limit of the watchdog reset time.
   */

  stm32_setwindow(priv, 0x7f);
  return OK;
}

/****************************************************************************
 * Name: stm32_capture
 *
 * Description:
 *   Don't reset on watchdog timer timeout; instead, call this user provider
 *   timeout handler.  NOTE:  Providing handler==NULL will restore the reset
 *   behavior.
 *
 * Input Parameters:
 *   lower      - A pointer the publicly visible representation of the
 *                "lower-half" driver state structure.
 *   newhandler - The new watchdog expiration function pointer.  If this
 *                function pointer is NULL, then the reset-on-expiration
 *                behavior is restored,
 *
 * Returned Value:
 *   The previous watchdog expiration function pointer or NULL is there was
 *   no previous function pointer, i.e., if the previous behavior was
 *   reset-on-expiration (NULL is also returned if an error occurs).
 *
 ****************************************************************************/

static xcpt_t stm32_capture(struct watchdog_lowerhalf_s *lower,
                            xcpt_t handler)
{
  struct stm32_lowerhalf_s *priv = (struct stm32_lowerhalf_s *)lower;
  irqstate_t flags;
  xcpt_t oldhandler;
  uint16_t regval;

  DEBUGASSERT(priv);
  wdinfo("Entry: handler=%p\n", handler);

  /* Get the old handler return value */

  flags = enter_critical_section();
  oldhandler = priv->handler;

  /* Save the new handler */

  priv->handler = handler;

  /* Are we attaching or detaching the handler? */

  regval = getreg32(STM32_WWDG_CFR);
  if (handler)
    {
      /* Attaching... Enable the EWI interrupt */

      regval |= WWDG_CFR_EWI;
      putreg32(regval, STM32_WWDG_CFR);

      up_enable_irq(STM32_IRQ_WWDG);
    }
  else
    {
      /* Detaching... Disable the EWI interrupt */

      regval &= ~WWDG_CFR_EWI;
      putreg32(regval, STM32_WWDG_CFR);

      up_disable_irq(STM32_IRQ_WWDG);
    }

  leave_critical_section(flags);
  return oldhandler;
}

/****************************************************************************
 * Name: stm32_ioctl
 *
 * Description:
 *   Any ioctl commands that are not recognized by the "upper-half" driver
 *   are forwarded to the lower half driver through this method.
 *
 * Input Parameters:
 *   lower - A pointer the publicly visible representation of the "lower-
 *           half" driver state structure.
 *   cmd   - The ioctl command value
 *   arg   - The optional argument that accompanies the 'cmd'.  The
 *           interpretation of this argument depends on the particular
 *           command.
 *
 * Returned Value:
 *   Zero on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int stm32_ioctl(struct watchdog_lowerhalf_s *lower, int cmd,
                       unsigned long arg)
{
  struct stm32_lowerhalf_s *priv = (struct stm32_lowerhalf_s *)lower;
  int ret = -ENOTTY;

  DEBUGASSERT(priv);
  wdinfo("Entry: cmd=%d arg=%ld\n", cmd, arg);

  /* WDIOC_MINTIME: Set the minimum ping time.  If two keepalive ioctls
   * are received within this time, a reset event will be generated.
   * Argument: A 32-bit time value in milliseconds.
   */

  if (cmd == WDIOC_MINTIME)
    {
      uint32_t mintime = (uint32_t)arg;

      /* The minimum time should be strictly less than the total delay
       * which, in turn, will be less than or equal to WWDG_CR_T_MAX
       */

      ret = -EINVAL;
      if (mintime < priv->timeout)
        {
          uint32_t window = (priv->timeout - mintime) * priv->fwwdg /
                            1000 - 1;
          DEBUGASSERT(window < priv->reload);
          stm32_setwindow(priv, window | WWDG_CR_T_RESET);
          ret = OK;
        }
    }

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_wwdginitialize
 *
 * Description:
 *   Initialize the WWDG watchdog timer.  The watchdog timer is initialized
 *   and registers as 'devpath'.  The initial state of the watchdog timer is
 *   disabled.
 *
 * Input Parameters:
 *   devpath - The full path to the watchdog.  This should be of the form
 *     /dev/watchdog0
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void stm32_wwdginitialize(const char *devpath)
{
  struct stm32_lowerhalf_s *priv = &g_wdgdev;

  wdinfo("Entry: devpath=%s\n", devpath);

  /* NOTE we assume that clocking to the WWDG has already been provided by
   * the RCC initialization logic.
   */

  /* Initialize the driver state structure.  Here we assume: (1) the state
   * structure lies in .bss and was zeroed at reset time.  (2) This function
   * is only called once so it is never necessary to re-zero the structure.
   */

  priv->ops = &g_wdgops;

  /* Attach our EWI interrupt handler (But don't enable it yet) */

  irq_attach(STM32_IRQ_WWDG, stm32_interrupt, NULL);

  /* Select an arbitrary initial timeout value.  But don't start the watchdog
   * yet. NOTE: If the "Hardware watchdog" feature is enabled through the
   * device option bits, the watchdog is automatically enabled at power-on.
   */

  stm32_settimeout((struct watchdog_lowerhalf_s *)priv, WWDG_MAXTIMEOUT);

  /* Register the watchdog driver as /dev/watchdog0 */

  watchdog_register(devpath, (struct watchdog_lowerhalf_s *)priv);

  /* When the microcontroller enters debug mode (Cortex-M core halted),
   * the WWDG counter either continues to work normally or stops, depending
   * on DBG_WWDG_STOP configuration bit in DBG module.
   */

#ifdef CONFIG_DEBUG_FEATURES
    {
      uint32_t cr = getreg32(STM32_DBGMCU_APB1_FZ);
      cr |= DBGMCU_APB1_WWDGSTOP;
      putreg32(cr, STM32_DBGMCU_APB1_FZ);
    }
#endif
}
