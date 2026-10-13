#!/usr/bin/env python3
"""Single source of truth for the MantisCalc guide.
Run:  python3 docs/guide_src.py
Generates src/guide_text.h (device), website/guide.html, USER_MANUAL.md (+ website copy).
Every recipe here is checked by test/test_main.cpp - change both together.
Notation: [KEY] = press that key.  2ND [x] = press B (or the 2ND bar), then the key whose
small lime label is x.  -> = what the display shows."""
import os, html
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

HOWTO = ("HOW THIS CALCULATOR WORKS", [
 "1 Normal math always works:   12 * 4 =",
 "2 STORE: type a number, press its key",
 "    120 [VOLTS]   tape line: = 120 V",
 "3 SOLVE: press a tool key ([OHM/PWR] [PMT])",
 "    it uses the numbers you stored",
 "4 Missing one? It beeps, or the top of the",
 "    display asks (V? flour? PV?): type it",
 "    and press the same tool key again",
 "5 2ND: press B, then a key. You get the",
 "    small lime word printed on that key",
 "6 Tap bottom of display: recent results",
 "  2ND [CLR] clears   A / C: other calculator",
 "Guide: tap right = next card, left = back"])

PAGES = [
 ("MATH", "scientific", [
  ("MATH  quick recipes", [
   "Fraction: 0.375 [FRAC]  -> tape 3/8",
   "  2.625 [FRAC] -> tape 21/8  (= 2 5/8)",
   "Power:   2 2ND [^] 10 =   -> 1024",
   "Root:    81 [sqrt]        -> 9",
   "Trig uses DEGREES:",
   "  30 [SIN] -> 0.5    0.5 2ND [asin] -> 30",
   "Tip 18 %:  85 * 18 2ND [%] =   -> 15.3",
   "Add 8 %:   49.99 + 8 2ND [%] = -> 53.99",
   "Big/small: 4.7 2ND [EE] 6   -> 4700000",
   "  tiny: 1 2ND [EE] 2ND [+/-] 6 -> 0.000001",
   "Memory: 2ND [MS] store   2ND [MR] recall"]),
  ("MATH  averages and lists", [
   "Average some measurements:",
   "  12.5 [STAT] 13.1 [STAT] 12.8 [STAT]",
   "  2ND [minMx] -> 12.5 (the smallest)",
   "  tap display bottom: max, sum, mean 12.8",
   "New list: 2ND [Sclr]",
   "Radio wavelength from frequency:",
   "  146 2ND [EE] 6 [WAVE] -> 2.053 m",
   "Engineering display (E3, E6): 2ND [ENG]"]),
  ("MATH  every key", [
   "[x^2]  square       2ND [sqrt] root",
   "[sqrt] root         2ND [x^2] square",
   "[1/x]  1 / x        2ND [%] x / 100",
   "[LOG]  log base 10  2ND [10^x]",
   "[LN]   natural log  2ND [e^x]",
   "[e^x]  e to the x   2ND [Sclr] clear STAT",
   "[SIN] [COS] [TAN]   2ND asin acos atan",
   "[WAVE] meters from Hz   2ND [f] store Hz",
   "[FRAC] fraction     2ND [ENG] display",
   "[STAT] add to list  2ND [minMx] results",
   "Pad 2ND: pi e % ^ BS +/- ABS mod 1/x",
   "  ANS EE d% CLR MR MS MC"])]),
 ("ELEC", "electrical", [
  ("ELEC  Ohm's law and power", [
   "Store any TWO of volts, amps, ohms, watts,",
   "then press [OHM/PWR] for the other two:",
   "  120 [VOLTS] 15 [AMPS] [OHM/PWR]",
   "      -> 8 ohm    tape: 1800 W",
   "  12 [VOLTS] 4 [OHMS] [OHM/PWR] -> 3 A",
   "  60 [WATTS] 120 [VOLTS] [OHM/PWR] -> 0.5 A",
   "The two you entered LAST are used.",
   "Small units: 2ND [mV] [mA] [kOhm] [uF] [mH]",
   "  4.7 2ND [kOhm] stores 4700 ohm",
   "Letters V A R P C L f d on the display",
   "  light up when that value is stored"]),
  ("ELEC  wire size and voltage drop", [
   "120 [VOLTS]   source voltage",
   "15 [AMPS]     load current",
   "75 [DIST]     ONE-WAY wire length, feet",
   "14 [AWG]      the wire you plan to use",
   "[VDROP]  -> 7.07 V lost in the wire",
   "  tape: 5.9 % drop, min gauge for 3 %",
   "Which wire do I need? Same first 3 lines,",
   "  then [AWG] with nothing typed -> 11",
   "  = thinnest for 3 % drop (buy 10 AWG)",
   "Aluminum: 2ND [Al/Cu]   (display shows Al)",
   "A calculator aid - always follow code."]),
  ("ELEC  filters, coils, AC", [
   "RC filter cutoff:",
   "  1000 [OHMS] 0.1 2ND [uF] [FILT]",
   "  -> 1591.5 Hz",
   "LC resonance:",
   "  10 2ND [mH] 100 2ND [uF] 2ND [RLC]",
   "  -> 159.2 Hz",
   "Coil reactance at 60 Hz:",
   "  60 [Hz] 10 2ND [mH] 2ND [XL/XC] -> 3.77",
   "Power factor: 900 [WATTS] 120 [VOLTS]",
   "  10 [AMPS] 2ND [PF] -> 0.75",
   "Gain: 2 2ND [dB] -> 6.02 dB"]),
  ("ELEC  every key", [
   "[VOLTS] [AMPS] [OHMS] [WATTS] store V A R P",
   "  2ND [mV] [mA] [kOhm]   WATTS 2ND [dB]",
   "[FARAD] store C      2ND [uF]",
   "[HENRY] store L      2ND [mH]",
   "[Hz] store freq.     2ND [wave] meters",
   "[DIST] wire feet     2ND [PF] power factor",
   "[OHM/PWR] solve      2ND [XL/XC] reactance",
   "[VDROP] wire drop    2ND [Al/Cu] metal",
   "[FILT] RC cutoff     2ND [RLC] resonance",
   "[AWG] gauge          2ND [cmil] wire area",
   "  [AWG] with nothing typed = best gauge"])]),
 ("MECH/RF", "motors, props, radio", [
  ("MECH/RF  motors and props", [
   "Motor top rpm (full charge, no load):",
   "  2300 [KV] 4 [CELLS] [MOTOR] -> 38640",
   "Prop pitch speed - type, press [PROP]:",
   "  2300 [PROP]  4 [PROP]   (KV, cells)",
   "  5 [PROP]  4.5 [PROP]    (diam in, pitch in)",
   "  -> 164.7 mph",
   "Battery: 2200 [BAT] 3 [BAT] -> 24.4 Wh",
   "  (mAh, cells)",
   "Pack volts: 4 2ND [packV] -> 14.8 V",
   "Thrust to weight: 1600 [THRUST]",
   "  800 [WEIGHT] 2ND [T/W] -> 2",
   "  tape: hover at 50 % throttle"]),
  ("MECH/RF  gears, power, radio", [
   "Gear output: 3000 [RPM] 3 [RATIO] [GEAR]",
   "  -> 1000 rpm",
   "Shaft power: 2 2ND [torq]  (N-m)",
   "  3000 [RPM] 2ND [Pwr] -> 628.3 W",
   "KV from measured rpm:",
   "  10000 [RPM] 3 [CELLS] 2ND [KV?] -> 793.7",
   "Antenna wavelength:",
   "  146 2ND [EE] 6 [FREQ] [WAVE] -> 2.053 m",
   "  2ND [ft] -> 6.737 ft",
   "rpm to rev/sec: 3000 2ND [rps] -> 50",
   "grams to lb: 1600 2ND [lb] -> 3.527"]),
  ("MECH/RF  every key", [
   "[KV] store KV         2ND [KV?] solve KV",
   "[CELLS] LiPo count    2ND [packV] volts",
   "[RPM] store rpm       2ND [rps] rpm / 60",
   "[THRUST] grams        2ND [lb] in pounds",
   "[WEIGHT] grams        2ND [T/W] ratio",
   "[RATIO] gear ratio    2ND [gear] = GEAR",
   "[PROP] pitch speed    2ND [mph] from RPM",
   "[GEAR] output rpm     2ND [Pwr] watts",
   "[FREQ] store Hz       2ND [wave] meters",
   "[WAVE] meters         2ND [ft] feet",
   "[BAT] mAh,S -> Wh     2ND [Wh] recompute",
   "[MOTOR] KV,S -> rpm   2ND [torq] store N-m"])]),
 ("BUILD", "construction", [
  ("BUILD  slopes, roofs, heights", [
   "Roof pitch 4 in 12:",
   "  4 [RISE] 12 [RUN] [SLOPE] -> 0.333",
   "  2ND [ang] -> 18.43 degrees",
   "Rafter: 4 [RISE] 12 [RUN] [ROOF] -> 12.65",
   "Height of a tree or pole by angle:",
   "  50 [RUN]     you stand 50 ft away",
   "  30 [ANGLE]   angle up to the top",
   "  2ND [rise] -> 28.87 ft  (+ eye height)",
   "Height by shadow (plain math):",
   "  your ht x its shadow / your shadow",
   "  6 * 40 / 8 =  -> 30 ft",
   "Square check: 3 [RISE] 4 [RUN] 2ND [hyp]",
   "  -> 5  (diagonal)"]),
  ("BUILD  stairs, lumber, concrete", [
   "Stairs: total rise floor-to-floor, FEET",
   "  9 [RISE] [STAIRS] -> 7.71 in per riser",
   "  tape: 14 risers, run, stringer length",
   "Board feet - type, press [BOARD]:",
   "  2 [BOARD] 6 [BOARD] 10 [BOARD] -> 10",
   "  (thick in, wide in, long ft)",
   "Concrete slab 10 x 10 ft, 4 in thick:",
   "  10 * 10 * 4 / 12 = [VOL] [CONC]",
   "  -> 1.36 yd3 (includes 10 % extra)",
   "Circle: 10 2ND [circ] -> 78.54 sq",
   "Post hole: 1 [RISE] 3 [RUN] 2ND [cyl]",
   "  -> 2.36 ft3  (diameter, depth)"]),
  ("BUILD  paint a room", [
   "Type each, press [PAINT] after each one:",
   "  48 [PAINT]   perimeter ft (12x12 room)",
   "  8 [PAINT]    wall height ft",
   "  1 [PAINT]    doors",
   "  2 [PAINT]    windows",
   "  2 [PAINT]    coats",
   "  350 [PAINT]  sq ft per gallon (can)",
   "  -> 1.90 gal    tape: buy 2",
   "Shortcut: after height press [PAINT]",
   "  with nothing typed: no doors/windows,",
   "  1 coat, 350 sq ft/gal",
   "The display top shows what PAINT wants."]),
  ("BUILD  every key", [
   "[RISE] store rise     2ND [slope] = SLOPE",
   "[RUN] store run       2ND [ang] angle",
   "[SLOPE] rise / run    2ND [ang] angle",
   "[ANGLE] store deg     2ND [rise] run x tan",
   "[AREA] store sq ft    2ND [circ] circle",
   "[VOL] store cu ft     2ND [cyl] cylinder",
   "[ROOF] rafter         2ND [hyp] diagonal",
   "[STAIRS] riser ht     2ND [#] riser count",
   "[BOARD] board ft      2ND [+10%] waste",
   "[PAINT] gallons       2ND [area] RISExRUN",
   "[CONC] yd3 + 10 %     2ND [yd3] no extra",
   "[TRIG] sin cos tan    2ND [inv] asin"])]),
 ("KITCHEN", "cooking and baking", [
  ("KITCHEN  bread and baking", [
   "Water for a dough:",
   "  500 [FLOUR] 70 [HYDR%] [BAKE] -> 350 g",
   "  or: 500 [BAKE] 70 [BAKE]",
   "Salt as baker's percent:",
   "  500 [FLOUR] 10 [TOTAL] 2ND [%] -> 2 %",
   "Grams per serving:",
   "  1200 [TOTAL] 8 [SERV] [RECIPE] -> 150",
   "Scale a recipe from 8 to 12 servings:",
   "  each amount: 250 * 12 / 8 =  -> 375",
   "Round pan area: 9 2ND [pan] -> 63.62",
   "  pan scale = new area / old area"]),
  ("KITCHEN  conversions", [
   "Cups to anything - type cups, then [CONV]",
   "  again and again:",
   "  2 [CONV] 2 cup  [CONV] 32 tbsp",
   "  [CONV] 96 tsp   [CONV] 473.2 mL",
   "  [CONV] 0.473 L  [CONV] 0.125 gal",
   "Oven C to F: 180 [TEMP] 2ND [C<>F] -> 356",
   "Oven F to C: 350 2ND [F] 2ND [C<>F]",
   "  -> 176.7",
   "8 [OZ>g] -> 226.8 g   250 2ND [g>OZ] -> 8.82",
   "2 [lb>kg] -> 0.907    1 2ND [kg>lb] -> 2.2",
   "Cost: 3.49 [PRICE] 4 [TOTAL] 2ND [cost]",
   "  -> 13.96"]),
  ("KITCHEN  every key", [
   "[FLOUR] grams        2ND [%] TOTAL as %",
   "[TOTAL] grams        2ND [serv] servings",
   "[HYDR%] water %      2ND [H2O] water g",
   "[SERV] servings      2ND [g/serv]",
   "[TEMP] store C       2ND [F] store F",
   "[PRICE] $ each       2ND [cost] PRICExTOTAL",
   "[BAKE] water grams   2ND [H2O] same",
   "[RECIPE] per serving 2ND [%] baker %",
   "[OZ>g] ounces->g     2ND [g>OZ]",
   "[CONV] cup chain     2ND [C<>F] temp",
   "[AREA] store         2ND [pan] round pan",
   "[lb>kg] pounds->kg   2ND [kg>lb]"])]),
 ("MONEY", "loans and savings", [
  ("MONEY  loans", [
   "Payment on $25,000 at 6.5 % for 5 years:",
   "  25000 [PRINC] 6.5 [RATE%] 60 [TERM]",
   "  [PMT] -> 489.15 / month",
   "  or: 25000 [PMT] 6.5 [PMT] 60 [PMT]",
   "How much can I borrow at $400/month?",
   "  400 2ND [set] 6.5 [RATE%] 60 [TERM]",
   "  2ND [PV] -> 20,442  (PV on PRINC key)",
   "Pay $100 extra every month:",
   "  after [PMT]: 100 2ND [extra] [PAYOFF]",
   "  -> 49 months (last one is partial)",
   "Balance after each month: [AMORT] [AMORT]",
   "  2ND [reset] starts over"]),
  ("MONEY  savings, percent, prices", [
   "Savings: $1000 at 5 % for 10 years",
   "  1000 [PRINC] 5 [RATE%] 120 [TERM]",
   "  [SAV] -> 1647.01",
   "Tip 18 %:  85 * 18 [%] =      -> 15.3",
   "Add 8 % tax: 49.99 + 8 [%] =  -> 53.99",
   "Change 80 -> 92:  80 - 92 2ND [d%] -> 15 %",
   "4 at $3.49: 3.49 [PRICE] 4 [QTY]",
   "  2ND [cost] -> 13.96",
   "Round to cents: [ROUND]   3 digits: 2ND [sig]",
   "Interest this month: 2ND [I/mo]"]),
  ("MONEY  every key", [
   "[PRINC] loan amount  2ND [PV] max loan",
   "[RATE%] yearly %     2ND [I/mo] month int.",
   "[TERM] months        2ND [n mo] to pay off",
   "[PMT] payment        2ND [set] store PMT",
   "[INT] month interest 2ND [total] PMT again",
   "[SAV] future value   2ND [FV] same",
   "[PAYOFF] months      2ND [extra] store $",
   "[AMORT] next month   2ND [reset]",
   "[%] percent          2ND [d%] % change",
   "[ROUND] cents        2ND [sig] 3 digits",
   "[PRICE] $ each       2ND [cost]",
   "[QTY] how many       2ND [cost]"])]),
]
MAXC = max(len(p[2]) for p in PAGES) + 1

def check():
    for name, _, cards in PAGES:
        for t, lines in [HOWTO] + cards:
            assert len(lines) <= 13, (t, len(lines))
            for l in [t] + lines:
                assert len(l) <= 50, (len(l), l)
                assert all(32 <= ord(c) < 127 for c in l), l

def gen_header():
    def c(s): return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'
    o = ["// GENERATED by docs/guide_src.py - edit that file, not this one.", "#pragma once",
         f"static const int GUIDE_MAXC={MAXC};", "struct GuideCard{const char*title;const char*l[13];};",
         f"static const int guideCount[6]={{{','.join(str(len(p[2])+1) for p in PAGES)}}};",
         f"static const GuideCard guide[6][{MAXC}]={{"]
    for name, _, cards in PAGES:
        o.append(" {")
        for t, lines in [HOWTO] + cards:
            o.append("  {" + c(t) + ",{" + ",".join(c(l) for l in lines) + "}},")
        o.append(" },")
    o.append("};")
    open(os.path.join(ROOT, "src/guide_text.h"), "w").write("\n".join(o) + "\n")

def gen_md():
    o = ["# MantisCalc User Guide", "", "_Generated from `docs/guide_src.py` - the same text is on the device "
         "(tap the top of the display)._", "",
         "**Notation:** `[KEY]` = press that key. `2ND [x]` = press **B** (or the 2ND bar), then the key whose "
         "small lime label is *x*. `->` = what the display shows.", "", f"## {HOWTO[0].title()}", "", "```"] + HOWTO[1] + ["```", ""]
    for name, desc, cards in PAGES:
        o += [f"## {name} - {desc}", ""]
        for t, lines in cards:
            o += [f"### {t}", "", "```"] + lines + ["```", ""]
    md = "\n".join(o)
    for p in ["USER_MANUAL.md", "website/USER_MANUAL.md"]:
        open(os.path.join(ROOT, p), "w").write(md)

def keyspan(line):
    import re
    s = html.escape(line)
    s = re.sub(r"2ND \[([^\]]+)\]", r'<span class="k2">2ND</span><span class="ks">\1</span>', s)
    s = re.sub(r"\[([^\]]+)\]", r'<span class="k">\1</span>', s)
    return s.replace("-&gt;", '<span class="ar">&rarr;</span>')

def gen_html():
    nav = "".join(f'<a href="#{n.split("/")[0].lower()}">{n}</a>' for n, _, _ in PAGES)
    body = [f'<section class="card how"><h2>{HOWTO[0].title()}</h2><pre>' + "\n".join(keyspan(l) for l in HOWTO[1]) + "</pre></section>"]
    for name, desc, cards in PAGES:
        body.append(f'<h2 class="pg" id="{name.split("/")[0].lower()}">{name} <small>{desc}</small></h2>')
        for t, lines in cards:
            body.append(f'<section class="card"><h3>{html.escape(t)}</h3><pre>' + "\n".join(keyspan(l) for l in lines) + "</pre></section>")
    page = f"""<!DOCTYPE html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MantisCalc Guide</title>
<!-- GENERATED by docs/guide_src.py - edit that file, not this one. -->
<style>
:root{{--bez:#2a3036;--lcd:#c4d4ba;--lcd2:#9eb296;--ink:#18241f;--teal:#0e4a4c;--lime:#c4e828;--purple:#5c0c66}}
*{{box-sizing:border-box}}body{{margin:0;font:15px/1.45 -apple-system,Segoe UI,Roboto,sans-serif;color:#e8ecef;
background:repeating-linear-gradient(0deg,#1d2226 0 1px,#22282d 1px 3px)}}
header{{padding:20px 16px;text-align:center;background:linear-gradient(#3a434b,#1b2024);border-bottom:2px solid #000}}
header h1{{margin:0;color:var(--lime);letter-spacing:2px;text-shadow:0 1px 0 #000}}
header p{{margin:6px 0 0;color:#b9c3cb}}
nav{{position:sticky;top:0;display:flex;flex-wrap:wrap;gap:6px;justify-content:center;padding:8px;background:#14181b;border-bottom:1px solid #000;z-index:2}}
nav a{{color:#fff;text-decoration:none;padding:5px 12px;border-radius:14px;background:linear-gradient(#4aa0a0,#0e4a4c);
box-shadow:0 2px 0 #031c1d,inset 0 1px 0 #8fd0d0;font-weight:600;font-size:13px}}
main{{max-width:760px;margin:0 auto;padding:16px}}
.legend{{background:#14181b;border:1px solid #000;border-radius:8px;padding:10px 14px;color:#cfd6dc}}
h2.pg{{margin:28px 0 10px;color:var(--lime);border-bottom:1px solid #444;padding-bottom:4px}}h2.pg small{{color:#9aa6ae;font-weight:400}}
.card{{background:linear-gradient(var(--lcd),var(--lcd2));color:var(--ink);border-radius:10px;padding:10px 14px 4px;margin:12px 0;
box-shadow:inset 0 2px 3px rgba(0,0,0,.45),0 1px 0 #6b7680;border:2px solid #0a0c0e}}
.card h2,.card h3{{margin:0 0 6px;font-size:14px;letter-spacing:1px;background:var(--ink);color:var(--lcd);display:inline-block;padding:2px 8px;border-radius:4px}}
pre{{margin:0 0 8px;font:14px/1.55 ui-monospace,Menlo,Consolas,monospace;white-space:pre-wrap}}
.k,.ks,.k2{{display:inline-block;padding:0 6px;border-radius:5px;font-weight:700;font-size:12px;line-height:18px;color:#fff;
background:linear-gradient(#4aa0a0,#0e4a4c);box-shadow:0 1px 0 #031c1d}}
.k2{{background:linear-gradient(#a24cb0,#46084e);margin-right:2px}}.ks{{color:var(--lime);background:linear-gradient(#3d6f71,#0b3a3c)}}
.ar{{font-weight:700}}footer{{text-align:center;color:#7d8891;padding:24px;font-size:13px}}
</style></head><body>
<header><h1>MANTISCALC</h1><p>Field guide - the same cards are on the calculator (tap the top of the display)</p></header>
<nav>{nav}</nav><main>
<p class="legend"><span class="k">KEY</span> press that key &nbsp; <span class="k2">2ND</span><span class="ks">x</span> press B (2ND), then the key with the small lime label x &nbsp; <b>&rarr;</b> what the display shows. Numbers in examples are checked by automated tests.</p>
{''.join(body)}
</main><footer>MantisCalc is a calculation aid. For electrical and structural work, follow your local code.</footer></body></html>"""
    open(os.path.join(ROOT, "website/guide.html"), "w").write(page)

if __name__ == "__main__":
    check(); gen_header(); gen_md(); gen_html(); print("guide generated:", sum(len(p[2]) for p in PAGES) + 6, "cards")
