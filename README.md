# MantisCalculator

A calculator-first, deterministic engineering/trades calculator for the **M5Stack Core2**.

It is designed to behave like a real physical calculator first: ordinary arithmetic is always available as `1 + 2 =`, while domain intelligence is layered behind unit-stamped registers and sequential function keys.

## Hardware

- M5Stack Core2
- ESP32-D0WDQ6-V3
- 16 MB flash / 8 MB PSRAM
- 320×240 capacitive touch display
- Built-in speaker and vibration motor

## UI

- **A**: previous function page
- **B**: latch/unlatch `2nd`
- **C**: next function page
- Function keys change with the page; numeric/operator keypad stays familiar.
- Secondary labels are shown on function keys and execute while `2nd` is latched.
- Domain values are stored as semantic registers. Enter a number, then press its unit/domain key.
- Sequential keys advance their routine on repeated presses, making multi-step engineering calculations possible without menus.

## Pages

### ELEC
Voltage, current, resistance, power, capacitance, inductance, frequency, distance, Ohm/power solving, voltage drop, RC/filter calculations, AWG, dB and wavelength.

### MECH/RF
Motor KV/cell/RPM relationships, thrust/weight, gear ratio, propeller speed/ground-speed approximation, battery energy, frequency/wavelength, torque-oriented motor calculations.

### BUILD
Rise/run/slope/angle, area/volume, roof geometry, stair stringer geometry, board feet, paint quantity, concrete quantity and trigonometry.

### KITCHEN
Flour/total/hydration/servings, baker-style water calculations, recipe scaling, temperature conversion, simple density/portion calculations and pan area/volume helpers.

### MONEY
Principal/rate/term/payment, interest, compound growth, extra-payment payoff, percentages, rounding and price/total arithmetic.

### MATH
Powers, roots, reciprocal, log/LN/exp, trig and inverse trig, wavelength/frequency, fraction conversion, rounding and basic statistic/tape helpers.

## Engineering notes

This firmware is deliberately deterministic and uses explicit equations/constants rather than cloud services or a general-purpose symbolic engine. Some trade calculations are intentionally estimates and should not be treated as electrical/building-code compliance engines. In particular, wire sizing is a calculator aid; verify applicable NEC/local/code requirements for actual installations.

Motor/propeller routines are first-order approximations, not prop-test substitutes. Paint, concrete, and material routines include explicit waste assumptions where documented by the function.

## Build (GitHub Actions only)

Firmware is **only** built by GitHub Actions. Do not rely on a local PlatformIO install.

1. Push to `main` (or run **Actions → Build MantisCalculator → Run workflow**).
2. After the run finishes:
   - **Artifacts** tab → download `MantisCalculator-firmware` (merged `.bin` + app + manifest).
   - **GitHub Pages** hosts the web installer (`esp-web-tools`) for USB flash from a browser.
3. Flash with the Pages install button, or `esptool.py write_flash 0x0 MantisCalculator.bin`.


## Full flash image

`merge_bin.py` creates `MantisCalculator.bin` using:

- bootloader: `0x1000`
- partitions: `0x8000`
- boot_app0: `0xE000`
- application: `0x10000`
- 16 MB flash
- QIO / 80 MHz

## Archive convention

The **source ZIP intentionally excludes `.github`**. Keep the GitHub Actions workflow outside the source archive when using the mobile unzip workflow. The repository ZIP contains `.github/workflows/ci.yml` for GitHub Pages deployment.
## Key-layer design

Every calculator key has a primary and secondary (`2nd`) role. The six function pages provide 72 domain keys; the fixed numeric/operator keypad provides 16 more secondary operations. B latches `2nd`, the selected secondary executes once, and the shift latch releases automatically. A/C remain page navigation controls.

The secondary layer is intentionally functional rather than decorative: conversions, inverse solves, alternate equations, constants, engineering notation, percentage delta, impedance/reactance, motor power, geometry, baking hydration, financial inverses, and scientific functions live there.

**Numeric 2nd layer (always available):** π e % ^ | BS ± ABS mod | 1/x ANS EE %Δ | CLR MR MS MC

## v1.7 skeuomorphic UI

- Brushed-gunmetal bezel, recessed LCD glass with 16-cell 7-segment readout + ghost segments, LCD annunciators (page, 2ND, M, ENG, Cu/Al, wizard step, V A R P C L f d).
- Glossy domed keys with drop shadow and 1px sink animation on press; shift legends printed on every key in lime and swapped when 2ND is latched.
- Palette from the MantisCalc logo: teal function keys, slate digits, purple operators, lime =. Soft-key pills name the prev/next calculator.
- Rendered to a PSRAM canvas and pushed in one blit (no flicker). Touch geometry unchanged.

## Fixes (v1.6)

- Touch: one action per tap (was auto-repeating) and the Core2 button strip no longer double-fires with A/B/C.
- Typing after a result starts a new number; `2 + sqrt 9 =` keeps its left operand; `0.1+0.2-0.3` shows 0; EE takes a negative exponent via +/-.
- Wizards take the number typed before the first press (`120 OHM/PWR 15 OHM/PWR` → 8 ohm, 1800 W) and can restart any time.
- New **HENRY / mH** key (XL/XC and RLC were unreachable). mV/mA/kOhm/uF/mH scale only their own register.
- CONV fixed on every page (page index was off by one). Al/Cu has its own key (VDROP 2ND); temp C↔F on CONV 2ND.
- AWG no longer recommends 40 ga when nothing fits; SLOPE shows the ratio; PAINT optional steps default on a bare press.
- ASCII-only device text; stronger haptic tick.
- `test/run_tests.sh`: 44 host-side checks of every major equation against the real source.

## Earlier fixes (v1.3)

Full multitool pass. All 2nd keys match labels. Unit engine, domain depth, UX flags.

- **Unit engine**: SI prefixes (mV/mA/kΩ/µF), page-aware CONVERT chains (kitchen cup→tbsp→tsp→mL→L→gal), Al/Cu wire toggle.
- **Fractions**: continued-fraction a/b (π→355/113). ENG notation toggle.
- **ELEC**: Ohm/Pwr, RC/RLC, VDROP+AWG (correct cmil), series/parallel accumulators, XL/XC, PF, LED helper, dB.
- **BUILD**: bidirectional slope/angle/rise, roof, stairs, board-ft, paint (room formula), concrete yd³+waste, circle/cyl.
- **KITCHEN**: baker % + water, recipe scale, volume chain, temp C↔F, pan area/vol.
- **MONEY**: loan PMT (verified $536.82), total interest, payoff+extra, amort step (balance+interest/mo), FV/PV.
- **MATH**: 10ˣ, trig+inv, FRAC, running STAT (n/min/max/Σ/μ).
- **UX**: register set-flags (V A R P C L f d), step tag in header, dirty redraw, CLR/BS/memory on numeric 2nd.
- Dense single file, section banners, TEST comments for agents.

