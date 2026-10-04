# MantisCalc User Manual

**M5Stack Core2 · Engineering & trades multitool**  
Built on GitHub Actions · No menus · Stamp units, then solve

---

## 1. Hardware & navigation

| Control | Action |
|---------|--------|
| **A** (touch bar or physical) | Previous page |
| **C** (touch bar or physical) | Next page |
| **B** (touch bar or physical) | Latch / unlatch **2ND** |
| Function keys (2 rows × 6) | Domain keys; small **2** mark = has secondary |
| Number pad | Digits and operators always available |
| **2ND** + number pad | CLR, BS, memory, constants, EE, % |

When **2ND** is on, the header shows a purple **2ND** badge and keys show their secondary labels (lime text).

**Pages (A/C):** MATH → ELEC → MECH/RF → BUILD → KITCHEN → MONEY → …

**On-device guide:** 2ND + tap header. Scroll upper/lower half. Exit bottom bar / B / CLR.

**Register flags** (header right): green pills **V A R P C L f d** appear when those registers are set.

**Tape:** last calculations scroll under the display value.

**Splash:** brand screen for ~1.5 s at boot.

---

## 2. Basic calculator (always)

| Key | Primary | 2ND |
|-----|---------|-----|
| 0–9 . | Enter digits / decimal | — |
| + - * / | Add subtract multiply divide | — |
| = | Complete pending operation | — |
| 2ND π | — | Insert π |
| 2ND e | — | Insert *e* |
| 2ND % | — | Value ÷ 100 |
| 2ND ^ | — | Power (yˣ); enter base, 2ND ^, exponent, = |
| 2ND BS | — | Backspace |
| 2ND +/- | — | Change sign |
| 2ND ABS | — | Absolute value |
| 2ND mod | — | Remainder (pending %) |
| 2ND 1/x | — | Reciprocal |
| 2ND ANS | — | Recall last answer (or memory) |
| 2ND EE | — | Scientific entry (`1e`) |
| 2ND d% | — | Percent change vs pending left operand |
| 2ND CLR | — | Clear entry, steps, operators |
| 2ND MR / MS / MC | — | Memory recall / store / clear |

Operators use **ASCII** `+` `-` `*` `/` `=` so they always render correctly on the Core2 font.

---

## 3. How unit stamping works

1. Type a number.  
2. Press a **unit key** (e.g. VOLTS). That stores the value in a named register and stamps its unit.  
3. Press a **solver** key (possibly repeatedly). Solvers read whichever registers are set and fill the missing quantity.

Example — Ohm’s law:

```
120  VOLTS
15   AMPS
OHM/PWR     → 8 ohm
OHM/PWR     → 1800 W
```

---

## 4. ELEC page

### Unit keys

| Primary | 2ND | Meaning |
|---------|-----|---------|
| **VOLTS** | mV | Stamp volts; 2ND scales current entry as millivolts into V register |
| **AMPS** | mA | Stamp amps; 2ND → milliamps |
| **OHMS** | kOhm | Stamp ohms; 2ND → kilohms |
| **WATTS** | dBm | Stamp watts; 2ND → 20·log₁₀ of magnitude (dB-ish) |
| **FARAD** | uF | Stamp farads; 2ND → microfarads |
| **Hz** | wave | Stamp frequency; 2ND → wavelength (c/f) in metres |

### Tool keys

| Primary | 2ND | How to use |
|---------|-----|------------|
| **DIST** | ft-in | Stamp one-way wire length (ft). 2ND converts ft↔in (and related length steps on CONVERT). |
| **OHM/PWR** | XL/XC | **Sequential.** Needs any two of V,I,R (or P). Press repeatedly: (1) solve missing V/I/R (2) power (3) R from P (4) V or I from P. **2ND:** reactance — needs Hz + L or C → XL or XC. |
| **VDROP** | Al/Cu | **Sequential.** Needs V (source), A, DIST (ft). Optional: stamp AWG into answer first. Steps: (1) voltage drop (2) % drop (3) AWG for ≤3% (4) AWG for ≤1%. **2ND on CONVERT while on ELEC** toggles copper vs aluminum (K=12.9 vs 21.2). Header shows `Al` when aluminum. |
| **FILT** | RLC | Needs R and C. Steps: (1) fc = 1/(2πRC) + tape τ (2) τ=RC (3) 0.693·RC. **2ND:** resonant f = 1/(2π√(LC)) — needs L and C. |
| **AWG** | cmil | With A+DIST+V set → recommended AWG for 3% + tape drop. Else enter gauge number → stamp AWG and tape circular mils. **2ND:** enter AWG → show cmil only. |
| **dB** | PF | Primary: 20·log₁₀(\|x\|). **2ND PF:** needs P, V, A → power factor P/(V·I). |

**Test vector:** 120 V, 15 A, 75 ft, 14 AWG → ~7.07 V drop, ~5.9%. Required AWG for 3% ≈ 10–11.

---

## 5. MECH / RF page

| Primary | 2ND | How to use |
|---------|-----|------------|
| **KV** | RPM | Stamp motor KV. 2ND recalls/uses RPM register relationship. |
| **CELLS** | xV | Stamp series cell count (LiPo “S”). |
| **RPM** | rps | Stamp RPM; 2ND → revolutions per second (÷60). |
| **THRUST** | lb | Stamp thrust (grams by default); 2ND → pounds. |
| **WEIGHT** | kg | Stamp weight (g); 2ND → kg. |
| **RATIO** | gear | Stamp gear ratio; 2ND runs gear solve (RPM/ratio or torque×ratio). |
| **PROP** | mph | Needs KV, CELLS, DIAM (and optional PITCH). Computes RPM and approx pitch speed (mph). 2ND: speed from RPM+PITCH. |
| **GEAR** | Pwr | Gear ratio solve. 2ND: mechanical power P = τ·ω from torque + RPM. |
| **FREQ** | wave | Same as Hz / wavelength. |
| **WAVE** | ft | Wavelength from frequency; 2ND length convert. |
| **BAT** | Wh | Capacity (mAh) + cells → energy Wh (3.7 V nominal). Or weight/thrust ratio. |
| **MOTOR** | torq | KV×cells×4.2 → RPM. 2ND stamps torque register from entry. |

---

## 6. BUILD page

| Primary | 2ND | How to use |
|---------|-----|------------|
| **RISE** | slope | Stamp rise (ft). 2ND runs slope sequential. |
| **RUN** | ang | Stamp run (ft). 2ND → angle = atan(rise/run) in degrees. |
| **SLOPE** | ang | Sequential: ratio → angle → percent. 2ND → angle. |
| **ANGLE** | rise | Stamp angle (deg). 2ND → rise = run · tan(angle). |
| **AREA** | circ | Stamp area. 2ND: circle area from diameter on entry (πd²/4). |
| **VOL** | cyl | Stamp volume. 2ND: cylinder volume — diameter from DIAM/RISE, height from RUN/DIST. |
| **ROOF** | hyp | Rafter/hyp length √(rise²+run²); tapes pitch angle. 2ND pure hypotenuse. |
| **STAIRS** | # | Stringer diagonal; tapes riser count. 2ND: floor(total rise / 7.5). |
| **BOARD** | +10% | Board feet: rise=thickness(in), run=width(in), dist=length(ft) → T·W·L/12. 2ND: ×1.10 waste. |
| **PAINT** | net | **Sequential.** RISE=perimeter(ft), RUN=height(ft), DIST=#doors, ANGLE=#windows. (1) net area = P·H − (doors·21 + wins·15). (2) TOTAL=coats, PRICE=coverage ft²/gal (default 350) → gallons + ceil buy. 2ND: rect area from rise×run. |
| **CONC** | yd3 | Volume (ft³) → yd³ with 10% waste (÷27 × 1.1). |
| **TRIG** | inv | Cycles SIN → COS → TAN of entry (degrees). 2ND: arcsin. |

**Test:** rise 12, run 16 → slope 0.75, angle ≈ 36.87°, 75%.

**Paint test:** perim 48, h 8, 1 door, 2 windows, 2 coats → net 333 ft², ≈1.90 gal.

---

## 7. KITCHEN page

| Primary | 2ND | How to use |
|---------|-----|------------|
| **FLOUR** | % | Stamp flour weight (g). 2ND: baker’s % of total vs flour. |
| **TOTAL** | /serv | Stamp total dough/mix weight. 2ND stamps servings from entry. |
| **HYDR%** | H2O | Stamp hydration %. 2ND: water mass = flour × hydr/100. |
| **SERV** | dens | Stamp servings. 2ND: density-like total/servings. |
| **TEMP** | F | Stamp °C. 2ND stamps as °F. |
| **PRICE** | x | Stamp unit price. 2ND: price × total. |
| **BAKE** | next | Sequential baker helpers: total → water from hydr → hydr from total → per serving. |
| **RECIPE** | scale | total/servings. 2ND recipe %. |
| **DENS** | g/cup | per-serving mass; 2ND CONVERT. |
| **CONV** | chain | **Volume chain** (press repeatedly): cup → tbsp → tsp → mL → L → gal. Also C↔F if temp set. |
| **AREA** | pan | Stamp area; 2ND round pan area from diameter. |
| **VOL** | pan | Stamp volume; 2ND cylinder/pan volume. |

---

## 8. MONEY page

| Primary | 2ND | How to use |
|---------|-----|------------|
| **PRINC** | PV | Stamp principal. 2ND: present value of annuity (needs PMT, TERM, RATE). |
| **RATE%** | I/mo | Annual interest %. 2ND: monthly interest only = P·r/12. |
| **TERM** | n mo | Number of months. 2ND: solve months from P, PMT, rate. |
| **PMT** | solve | Runs loan payment solve (needs PRINC, RATE, TERM). 2ND starts amort step mode. |
| **INT** | total | Monthly interest-only; 2ND re-runs full loan (payment + total interest). |
| **SAV** | FV | Compound growth FV; 2ND same FV. |
| **PAYOFF** | extra | Months to pay off with optional EXTRA payment. |
| **AMORT** | step | Each press: balance after N months + interest component that month. |
| **%** | d% | ÷100; 2ND percent delta. |
| **ROUND** | sig | 2 decimal places; 2ND 3 significant figures. |
| **PRICE** / **TOTAL** | x / sum | Line totals / cashflow. |

**Test:** $100 000, 5%, 360 months → PMT ≈ **$536.82**.

---

## 9. MATH page

| Primary | 2ND | How to use |
|---------|-----|------------|
| **x^2** | sqrt | Square / square root |
| **sqrt** | x^2 | Square root / square |
| **1/x** | % | Reciprocal / percent |
| **LOG** | 10^x | log₁₀ / antilog 10ˣ |
| **LN** | e^x | Natural log / exp |
| **e^x** | LN | exp / ln |
| **SIN COS TAN** | asin acos atan | Degrees in, degrees out for inverses |
| **WAVE** | f | Wavelength ↔ frequency |
| **FRAC** | ENG | Best rational a/b (den ≤ 10 000). Tape shows `355/113` for π. **2ND:** engineering notation toggle on display. |
| **STAT** | minMx | Each press adds sample (n, min, max, Σ). 2ND reports min/max/Σ/mean. CLR clears stats with entry clear. |

---

## 10. Workflow recipes

**Wire sizing**  
`source V` → VOLTS · `amps` → AMPS · `one-way ft` → DIST · optional gauge → AWG · VDROP ×3 for drop / % / required AWG. Toggle Al with CONVERT 2ND on ELEC.

**RC low-pass**  
`R` → OHMS · `C` → FARAD (or uF via 2ND) · FILT.

**Loan**  
`principal` → PRINC · `apr` → RATE% · `months` → TERM · PMT.

**Baker’s water**  
`flour g` → FLOUR · `hydration %` → HYDR% · BAKE (or HYDR% 2ND).

**Roof rafter**  
`rise` → RISE · `run` → RUN · ROOF.

**Fraction**  
Enter decimal · MATH · FRAC · read tape `num/den`.

---

## 11. Notes & limits

- Wire drop uses DC copper (or Al) approximation; **not** a substitute for NEC / local code.
- Motor/prop helpers are first-order estimates.
- Paint/concrete include simple waste factors where noted.
- Default font is ASCII-only for labels so keys stay readable; unit tape uses `ohm`, `deg`, `yd3`, etc.
- Firmware is produced **only** by GitHub Actions; Pages hosts USB install + merged `.bin` download.

---

## 12. Brand

| Color | Hex | Role |
|-------|-----|------|
| Teal | `#007373` | Primary keys / headers |
| Purple | `#5d005d` | 2ND mode |
| Lime | `#c4e000` | Accents / secondary labels |

