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
  printf("%d passed, %d failed\n",passes,fails);return fails!=0;
}
