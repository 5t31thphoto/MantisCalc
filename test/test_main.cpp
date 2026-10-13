// Host tests: g++ -std=gnu++17 -Imock test_main.cpp && ./a.out
#include "../src/main.cpp"
static int fails=0,passes=0;
static void type(const char*s){for(;*s;s++){if(*s>='0'&&*s<='9')digit(*s-'0');else if(*s=='.')dot();else if(*s=='e')doAct(A_EE);else if(*s=='-')sign();}}
static void chk(const char*name,double got,double want,double tol=1e-6){
  bool ok=fabs(got-want)<=tol*fmax(1,fabs(want))||(isnan(want)&&isnan(got));
  if(ok)passes++;else{fails++;printf("FAIL %-28s got %.10g want %.10g\n",name,got,want);}}
static void reset(){reg=Registers();memory=0;stN=0;page=0;useAl=false;clearEntry();}
static void K(int pg,int i,bool sec=false){page=pg;Key&k=pages[pg][i];doAct(sec?k.b:k.a);}
int main(){
  initAwg();
  // arithmetic
  reset();type("12");doAct(A_MUL);type("4");equals();chk("12*4",curValue(),48);
  type("5");chk("typing after result starts new number",curValue(),5);
  reset();type("2");doAct(A_ADD);type("9");doAct(A_SQRT);equals();chk("2+sqrt9",curValue(),5);
  reset();type("0.1");doAct(A_ADD);type("0.2");doAct(A_SUB);type("0.3");equals();chk("0.1+0.2-0.3",curValue(),0,0);
  reset();type("4.7e");sign();type("6");chk("EE neg exponent",curValue(),4.7e-6,1e-12);
  reset();type("2");doAct(A_POW);type("10");equals();chk("2^10",curValue(),1024);
  reset();type("17");doAct(A_MOD);type("5");equals();chk("17 mod 5",curValue(),2);
  reset();type("50");doAct(A_SUB);type("60");doAct(A_PERCENTDELTA);chk("d% 50->60",curValue(),20);
  // MATH
  reset();type("30");K(0,6);chk("sin30",curValue(),0.5);
  reset();type("0.5");K(0,6,true);chk("asin.5",curValue(),30);
  reset();type("100");K(0,3);chk("log100",curValue(),2);
  reset();type("2");K(0,3,true);chk("10^2",curValue(),100);
  reset();type("0.375");K(0,10);chk("frac keeps value",curValue(),0.375);if(strcmp(tape[0],"3/8"))printf("FAIL frac tape %s\n",tape[0]),fails++;
  reset();type("2");K(0,11);type("4");K(0,11);type("9");K(0,11);K(0,11,true);chk("stat min",curValue(),2);
  // ELEC: wizard per guide: 120 OHM/PWR 15 OHM/PWR -> 8 ohm, 1800 W
  reset();page=1;type("120");K(1,7);type("15");K(1,7);chk("ohm wizard R",reg.r.x,8);chk("ohm wizard P",reg.p.x,1800);
  reset();page=1;type("12");K(1,0);type("4");K(1,2);K(1,7);chk("stamp V,R -> I",reg.a.x,3);
  reset();page=1;type("15");K(1,1,true);chk("mA",reg.a.x,0.015);chk("mA leaves V",reg.v.set,0);
  reset();page=1;type("100");K(1,9);type("1e");sign();type("6");K(1,9);chk("FILT fc",reg.hz.x,1591.549431,1e-6);
  reset();page=1;type("10");K(1,11,true);type("100");K(1,4,true);K(1,9,true);chk("RLC f0 (10mH,100uF)",reg.hz.x,159.1549431,1e-6);
  reset();page=1;type("60");K(1,5);type("10");K(1,11,true);K(1,7,true);chk("XL 60Hz 10mH",reg.answer.x,3.769911184,1e-6);
  reset();page=1;type("120");K(1,8);type("15");K(1,8);type("75");K(1,8);type("14");K(1,8);chk("VDROP 14awg",reg.answer.x,7.067,2e-3);
  reset();page=1;type("12");K(1,8);type("100");K(1,8);type("100");K(1,8);type("12");K(1,8);
  if(!strstr(tape[0],"ERR"))printf("FAIL awg impossible should be ERR: %s\n",tape[0]),fails++;else passes++;
  reset();page=1;K(1,8,true);chk("Al toggle",useAl,1);
  // MECH
  reset();page=2;type("1000");K(2,11);type("4");K(2,11);chk("MOTOR rpm",reg.rpm.x,16800);
  reset();page=2;type("2200");K(2,10);type("3");K(2,10);chk("BAT Wh",reg.energy.x,24.42);
  // BUILD
  reset();page=3;type("12");K(3,2);type("16");K(3,2);chk("slope",reg.answer.x,0.75);chk("angle",reg.angle.x,36.86989765);
  reset();page=3;type("12");K(3,6);type("16");K(3,6);chk("roof rafter",reg.answer.x,20);
  reset();page=3;type("2");K(3,8);type("6");K(3,8);type("10");K(3,8);chk("board ft",reg.answer.x,10);
  reset();page=3;for(const char*v:{"48","8","1","2","2","350"}){type(v);K(3,9);}chk("paint gal",reg.answer.x,333.0*2/350);
  reset();page=3;type("48");K(3,9);type("8");K(3,9);K(3,9);chk("paint defaults",reg.answer.x,384.0/350);
  reset();page=3;type("270");K(3,5);K(3,10);chk("concrete yd3",reg.answer.x,11.0);
  // KITCHEN
  reset();page=4;type("1000");K(4,6);type("65");K(4,6);chk("bake water",reg.answer.x,650);
  reset();page=4;type("2");K(4,9);chk("conv cup->cup",reg.answer.x,2);K(4,9);chk("conv ->tbsp",reg.answer.x,32);
  reset();page=4;type("100");K(4,9,true);chk("C->F",reg.temp.x,212);K(4,9,true);chk("F->C",reg.temp.x,100);
  // MONEY: 100000 PMT 5 PMT 360 PMT -> 536.82
  reset();page=5;type("100000");K(5,3);type("5");K(5,3);type("360");K(5,3);chk("PMT",reg.payment.x,536.8216,1e-6);
  K(5,6);chk("payoff months",reg.term.x,360,0);
  reset();page=5;type("100000");K(5,0);type("5");K(5,1);type("360");K(5,2);K(5,3);K(5,7);chk("amort bal mo1",reg.answer.x,99879.85,1e-6);
  // touch: tap below screen (Core2 button zone) must not act
  M5.Touch.d={160,260,true};shift=false;touch();chk("button-zone touch ignored",shift,0);
  M5.Touch.d={160,230,true};touch();chk("bar B toggles 2nd",shift,1);
  // ===== every example printed in docs/guide_src.py =====
  #define D curValue()
  // MATH
  reset();type("2.625");K(0,10);if(strcmp(tape[0],"21/8"))printf("FAIL frac 21/8 %s\n",tape[0]),fails++;
  reset();type("2");doAct(A_POW);type("10");equals();chk("g 2^10",D,1024);
  reset();type("81");K(0,1);chk("g sqrt81",D,9);
  reset();type("85");doAct(A_MUL);type("18");doAct(A_PERCENT);equals();chk("g tip",D,15.3);
  reset();type("49.99");doAct(A_ADD);type("8");doAct(A_PERCENT);equals();chk("g +8%",D,53.9892);
  reset();type("4.7");doAct(A_EE);type("6");chk("g EE",D,4.7e6);
  reset();type("1");doAct(A_EE);sign();type("6");chk("g EE-",D,1e-6,1e-9);
  reset();for(const char*v:{"12.5","13.1","12.8"}){type(v);K(0,11);}K(0,11,true);chk("g stat min",D,12.5);
  chk("g stat mean",stSum/stN,12.8);
  reset();type("146");doAct(A_EE);type("6");K(0,9);chk("g wave",D,2.0534,1e-4);
  // ELEC
  reset();page=1;type("12");K(1,0);type("4");K(1,2);K(1,7);chk("g 12V 4ohm",D,3);
  reset();page=1;type("60");K(1,3);type("120");K(1,0);K(1,7);chk("g 60W 120V I",D,0.5);chk("g R",reg.r.x,240);
  reset();page=1;type("120");K(1,0);type("15");K(1,1);K(1,7);chk("g 8ohm",D,8);chk("g 1800W",reg.p.x,1800);
  type("10");K(1,2);K(1,7);chk("g newest two (A,R) -> V",reg.v.x,150);
  reset();page=1;type("4.7");K(1,2,true);chk("g kOhm",reg.r.x,4700);
  reset();page=1;type("120");K(1,0);type("15");K(1,1);type("75");K(1,6);type("14");K(1,10);K(1,8);chk("g vdrop",D,7.067,2e-3);
  reset();page=1;type("120");K(1,0);type("15");K(1,1);type("75");K(1,6);K(1,10);chk("g best awg",D,11);
  reset();page=1;type("1000");K(1,2);type("0.1");K(1,4,true);K(1,9);chk("g filt",D,1591.549,1e-5);
  reset();page=1;type("10");K(1,11,true);type("100");K(1,4,true);K(1,9,true);chk("g rlc",D,159.155,1e-5);
  reset();page=1;type("60");K(1,5);type("10");K(1,11,true);K(1,7,true);chk("g XL",D,3.7699,1e-4);
  reset();page=1;type("900");K(1,3);type("120");K(1,0);type("10");K(1,1);K(1,6,true);chk("g PF",D,0.75);
  reset();page=1;type("2");K(1,3,true);chk("g dB",D,6.0206,1e-4);
  // MECH
  reset();page=2;type("2300");K(2,0);type("4");K(2,1);K(2,11);chk("g motor",D,38640);
  reset();page=2;for(const char*v:{"2300","4","5","4.5"}){type(v);K(2,6);}chk("g prop mph",D,164.659,1e-5);
  reset();page=2;type("2200");K(2,10);type("3");K(2,10);chk("g bat",D,24.42);
  reset();page=2;type("4");K(2,1,true);chk("g packV",D,14.8);
  reset();page=2;type("1600");K(2,3);type("800");K(2,4);K(2,4,true);chk("g T/W",D,2);
  reset();page=2;type("3000");K(2,2);type("3");K(2,5);K(2,7);chk("g gear",D,1000);
  reset();page=2;type("2");K(2,11,true);type("3000");K(2,2);K(2,7,true);chk("g power",D,628.3185,1e-6);
  reset();page=2;type("10000");K(2,2);type("3");K(2,1);K(2,0,true);chk("g KV?",D,793.65,1e-5);
  reset();page=2;type("146");doAct(A_EE);type("6");K(2,8);K(2,9);chk("g wave m",D,2.0534,1e-4);K(2,9,true);chk("g wave ft",D,6.737,1e-4);
  reset();page=2;type("3000");K(2,2,true);chk("g rps",D,50);
  reset();page=2;type("1600");K(2,3,true);chk("g lb",D,3.5274,1e-4);
  // BUILD
  reset();page=3;type("4");K(3,0);type("12");K(3,1);K(3,2);chk("g slope",D,1.0/3);K(3,1,true);chk("g ang",D,18.4349,1e-5);
  reset();page=3;type("4");K(3,0);type("12");K(3,1);K(3,6);chk("g rafter",D,12.6491,1e-5);
  reset();page=3;type("50");K(3,1);type("30");K(3,3);K(3,3,true);chk("g tree",D,28.8675,1e-5);
  reset();type("6");doAct(A_MUL);type("40");doAct(A_DIV);type("8");equals();chk("g shadow",D,30);
  reset();page=3;type("3");K(3,0);type("4");K(3,1);K(3,6,true);chk("g hyp",D,5);
  reset();page=3;type("9");K(3,0);K(3,7);chk("g stairs",D,108.0/14);
  reset();page=3;for(const char*v:{"2","6","10"}){type(v);K(3,8);}chk("g board",D,10);
  reset();page=3;type("10");doAct(A_MUL);type("10");doAct(A_MUL);type("4");doAct(A_DIV);type("12");equals();K(3,5);K(3,10);chk("g conc",D,1.3580,1e-4);
  reset();page=3;type("10");K(3,4,true);chk("g circ",D,78.5398,1e-5);
  reset();page=3;type("1");K(3,0);type("3");K(3,1);K(3,5,true);chk("g cyl",D,2.3562,1e-4);
  reset();page=3;for(const char*v:{"48","8","1","2","2","350"}){type(v);K(3,9);}chk("g paint",D,1.9029,1e-4);
  reset();page=3;for(const char*v:{"48","8","0","0","1","350"}){type(v);K(3,9);}chk("g paint zeros typed",D,384.0/350);
  // KITCHEN
  reset();page=4;type("500");K(4,0);type("70");K(4,2);K(4,6);chk("g bake",D,350);
  reset();page=4;type("500");K(4,6);type("70");K(4,6);chk("g bake wiz",D,350);
  reset();page=4;type("500");K(4,0);type("10");K(4,1);K(4,0,true);chk("g baker%",D,2);
  reset();page=4;type("1200");K(4,1);type("8");K(4,3);K(4,7);chk("g recipe",D,150);
  reset();type("250");doAct(A_MUL);type("12");doAct(A_DIV);type("8");equals();chk("g scale",D,375);
  reset();page=4;type("9");K(4,10,true);chk("g pan",D,63.617,1e-5);
  reset();page=4;type("2");K(4,9);chk("g c1",D,2);K(4,9);chk("g tbsp",D,32);K(4,9);chk("g tsp",D,96);
  K(4,9);chk("g mL",D,473.176,1e-5);K(4,9);chk("g L",D,0.473176,1e-5);K(4,9);chk("g gal",D,0.125);
  reset();page=4;type("180");K(4,4);K(4,9,true);chk("g C>F",D,356);
  reset();page=4;type("350");K(4,4,true);K(4,9,true);chk("g F>C",D,176.667,1e-5);
  reset();page=4;type("8");K(4,8);chk("g oz>g",D,226.796,1e-5);type("250");K(4,8,true);chk("g g>oz",D,8.8185,1e-4);
  reset();page=4;type("2");K(4,11);chk("g lb>kg",D,0.90718,1e-4);type("1");K(4,11,true);chk("g kg>lb",D,2.20462,1e-5);
  reset();page=4;type("3.49");K(4,5);type("4");K(4,1);K(4,5,true);chk("g cost",D,13.96);
  // MONEY
  reset();page=5;type("25000");K(5,0);type("6.5");K(5,1);type("60");K(5,2);K(5,3);chk("g pmt",D,489.15,1e-5);
  type("100");K(5,6,true);K(5,6);chk("g payoff mo",D,49,0);chk("g term kept",reg.term.x,60);
  reset();page=5;for(const char*v:{"25000","6.5","60"}){type(v);K(5,3);}chk("g pmt wiz",D,489.15,1e-5);
  reset();page=5;type("400");K(5,3,true);type("6.5");K(5,1);type("60");K(5,2);K(5,0,true);chk("g PV",D,20442,1e-4);
  reset();page=5;type("1000");K(5,0);type("5");K(5,1);type("120");K(5,2);K(5,5);chk("g sav",D,1647.01,1e-6);
  reset();page=5;type("85");doAct(A_MUL);type("18");K(5,8);equals();chk("g tip $",D,15.3);
  reset();page=5;type("80");doAct(A_SUB);type("92");K(5,8,true);chk("g d%",D,15);equals();chk("g d% no dangling op",D,15);
  reset();page=5;type("3.49");K(5,10);type("4");K(5,11);K(5,11,true);chk("g qty cost",D,13.96);
  printf("%d passed, %d failed\n",passes,fails);return fails!=0;
}
