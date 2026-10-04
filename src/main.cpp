#include <M5Unified.h>
#include <M5GFX.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "logo_rgb565.h"

// MantisCalculator v1.5 — full multitool. Dense. Agent-maintained.
// A/C=pages B=2nd. Stamp units → solvers. No menus. No long-press.
// === SECTIONS: consts | regs | keys | core | unit | elec | mech | build | kit | money | math | ui ===

static constexpr int W=320,H=240,TOP=36,TAPE_Y=36,TAPE_H=18,FUNC_Y=56,NUM_Y=128,BAR_Y=218;
static constexpr double PI_D=3.14159265358979323846,C_LIGHT=299792458.0,K_CU=12.9,K_AL=21.2,G_ACC=9.80665;

// === REGS ===
enum Unit{U_NONE,U_V,U_A,U_OHM,U_W,U_F,U_H,U_HZ,U_M,U_FT,U_IN,U_YD,U_DEG,U_RAD,U_PCT,
  U_G,U_KG,U_LB,U_RPM,U_KV,U_CELL,U_C,U_FAH,U_CUP,U_TBSP,U_TSP,U_OZFL,U_DOLLAR,
  U_GAL,U_LITER,U_AWG,U_CMIL,U_YD3,U_ML,U_WH};
struct Value{double x=0;Unit u=U_NONE;bool set=false;};
struct Registers{
  Value v,a,r,p,c,l,hz,dist,rise,run,angle,area,volume;
  Value rpm,kv,cells,thrust,weight,ratio,pitch,diameter,torque,energy,capacity;
  Value flour,total,hydration,servings,temp,price;
  Value principal,rate,term,payment,interest,savings,extra;
  Value answer;
}reg;

static int stN=0;static double stMin=0,stMax=0,stSum=0;
static double cur=0,memory=0,awgCm[41];
static bool entering=false,shift=false,dirty=true,engMode=false,useAl=false;
static char op=0,entry[40]="0";static int entryLen=1,page=0,tapeCount=0;
static char tape[8][48];
static uint8_t stepOhm=0,stepFilt=0,stepVD=0,stepSlope=0,stepTrig=0,stepBake=0,stepPaint=0,stepAmort=0,stepConv=0;
static unsigned long lastTouch=0;
static const char* stepTag=""; // shown in header when sequential active

enum Act{
  A_NONE,A_V,A_A,A_R,A_P,A_C,A_L,A_HZ,A_DIST,A_RISE,A_RUN,A_ANGLE,A_AREA,A_VOLUME,
  A_KV,A_CELLS,A_RPM,A_THRUST,A_WEIGHT,A_RATIO,A_PITCH,A_DIAM,A_TORQUE,A_ENERGY,A_CAPACITY,
  A_FLOUR,A_TOTAL,A_HYDR,A_SERV,A_TEMP,A_TEMPF,A_PRICE,A_PRINC,A_RATE,A_TERM,A_EXTRA,
  A_ADD,A_SUB,A_MUL,A_DIV,A_EQ,A_DOT,A_SIGN,A_BS,A_CLR,A_MC,A_MR,A_MS,A_MPLUS,
  A_OHMPWR,A_FILT,A_VDROP,A_AWG,A_SERIES,A_PARALLEL,A_RC,A_RLC,A_LED,A_DB,A_WAVELEN,
  A_GEAR,A_PROP,A_BATTERY,A_MOTOR,A_SLOPE,A_ROOF,A_STAIRS,A_BOARD,A_PAINT,A_CONCRETE,A_TRIG,
  A_BAKE,A_RECIPE,A_DENSITY,A_CONVERT,A_LOAN,A_AMORT,A_COMPOUND,A_PAYOFF,A_PERCENT,
  A_SQRT,A_SQUARE,A_RECIP,A_LOG,A_LN,A_EXP,A_POW10,A_SIN,A_COS,A_TAN,A_ASIN,A_ACOS,A_ATAN,
  A_FRAC,A_ROUND,A_CEIL,A_FLOOR,A_STAT,A_MINMAX,A_PI,A_E,A_ANS,A_ABS,A_POW,A_MOD,A_EE,
  A_PERCENTDELTA,A_CIRC,A_REACT,A_PF,A_SERIESC,A_PARALLELC,A_CAPENERGY,A_BATTERYWH,
  A_TORQUEPOWER,A_SPEED,A_FORCE,A_MASS,A_RECTAREA,A_CIRCLEAREA,A_CYLVOL,A_PYTH,
  A_STAIRCOUNT,A_MATERIALWASTE,A_PANROUND,A_HYDRWATER,A_RECIPEPCT,A_FV,A_INTEREST,
  A_PAYMONTHS,A_PRESENT,A_CASHFLOW,A_DEG2RAD,A_RAD2DEG,A_SINH,A_COSH,A_TANH,A_SIGFIG,
  A_SIPREFIX,A_CMIL,A_ANGLE_SOLVE,A_RISE_FROM_ANGLE,A_ENG,A_STATCLR,A_SERIESR,A_PARALLELR
};
struct Key{const char* p;const char* s;Act a,b;};

static const char* pageNames[]={"ELEC","MECH/RF","BUILD","KITCHEN","MONEY","MATH"};
static const uint16_t accents[]={0x038E,0x580B,0xAF27,0x0451,0x780F,0xC700}; // teal purple lime teal2 purple2 lime

// === KEYS: every secondary label matches its action ===
static Key pages[6][12]={
 {{"VOLTS","mV",A_V,A_SIPREFIX},{"AMPS","mA",A_A,A_SIPREFIX},{"OHMS","kOhm",A_R,A_SIPREFIX},{"WATTS","dBm",A_P,A_DB},{"FARAD","uF",A_C,A_SIPREFIX},{"Hz","wave",A_HZ,A_WAVELEN},
  {"DIST","ft-in",A_DIST,A_CONVERT},{"OHM/PWR","XL/XC",A_OHMPWR,A_REACT},{"VDROP","Al/Cu",A_VDROP,A_CONVERT},{"FILT","RLC",A_FILT,A_RLC},{"AWG","cmil",A_AWG,A_CMIL},{"dB","PF",A_DB,A_PF}},
 {{"KV","RPM",A_KV,A_RPM},{"CELLS","xV",A_CELLS,A_CONVERT},{"RPM","rps",A_RPM,A_CONVERT},{"THRUST","lb",A_THRUST,A_CONVERT},{"WEIGHT","kg",A_WEIGHT,A_CONVERT},{"RATIO","gear",A_RATIO,A_GEAR},
  {"PROP","mph",A_PROP,A_SPEED},{"GEAR","Pwr",A_GEAR,A_TORQUEPOWER},{"FREQ","wave",A_HZ,A_WAVELEN},{"WAVE","ft",A_WAVELEN,A_CONVERT},{"BAT","Wh",A_BATTERY,A_BATTERYWH},{"MOTOR","torq",A_MOTOR,A_TORQUE}},
 {{"RISE","slope",A_RISE,A_SLOPE},{"RUN","ang",A_RUN,A_ANGLE_SOLVE},{"SLOPE","ang",A_SLOPE,A_ANGLE_SOLVE},{"ANGLE","rise",A_ANGLE,A_RISE_FROM_ANGLE},{"AREA","circ",A_AREA,A_CIRCLEAREA},{"VOL","cyl",A_VOLUME,A_CYLVOL},
  {"ROOF","hyp",A_ROOF,A_PYTH},{"STAIRS","#",A_STAIRS,A_STAIRCOUNT},{"BOARD","+10%",A_BOARD,A_MATERIALWASTE},{"PAINT","net",A_PAINT,A_RECTAREA},{"CONC","yd3",A_CONCRETE,A_CONVERT},{"TRIG","inv",A_TRIG,A_ASIN}},
 {{"FLOUR","%",A_FLOUR,A_RECIPEPCT},{"TOTAL","/serv",A_TOTAL,A_SERV},{"HYDR%","H2O",A_HYDR,A_HYDRWATER},{"SERV","dens",A_SERV,A_DENSITY},{"TEMP","F",A_TEMP,A_TEMPF},{"PRICE","x",A_PRICE,A_TOTAL},
  {"BAKE","next",A_BAKE,A_HYDRWATER},{"RECIPE","scale",A_RECIPE,A_RECIPEPCT},{"DENS","g/cup",A_DENSITY,A_CONVERT},{"CONV","chain",A_CONVERT,A_SIPREFIX},{"AREA","pan",A_AREA,A_PANROUND},{"VOL","pan",A_VOLUME,A_CYLVOL}},
 {{"PRINC","PV",A_PRINC,A_PRESENT},{"RATE%","I/mo",A_RATE,A_INTEREST},{"TERM","n mo",A_TERM,A_PAYMONTHS},{"PMT","solve",A_LOAN,A_AMORT},{"INT","total",A_INTEREST,A_LOAN},{"SAV","FV",A_COMPOUND,A_FV},
  {"PAYOFF","extra",A_PAYOFF,A_PAYMONTHS},{"AMORT","step",A_AMORT,A_CASHFLOW},{"%","d%",A_PERCENT,A_PERCENTDELTA},{"ROUND","sig",A_ROUND,A_SIGFIG},{"PRICE","x",A_PRICE,A_TOTAL},{"TOTAL","sum",A_TOTAL,A_CASHFLOW}},
 {{"x^2","sqrt",A_SQUARE,A_SQRT},{"sqrt","x^2",A_SQRT,A_SQUARE},{"1/x","%",A_RECIP,A_PERCENT},{"LOG","10^x",A_LOG,A_POW10},{"LN","e^x",A_LN,A_EXP},{"e^x","LN",A_EXP,A_LN},
  {"SIN","asin",A_SIN,A_ASIN},{"COS","acos",A_COS,A_ACOS},{"TAN","atan",A_TAN,A_ATAN},{"WAVE","f",A_WAVELEN,A_HZ},{"FRAC","ENG",A_FRAC,A_ENG},{"STAT","minMx",A_STAT,A_MINMAX}}
};

// === CORE ===
static const char* us(Unit u){
  static const char* n[]={"","V","A","ohm","W","F","H","Hz","m","ft","in","yd","deg","rad","%","g","kg","lb","rpm","KV","S","C","F","cup","tbsp","tsp","floz","$","gal","L","AWG","cmil","yd3","mL","Wh"};
  return (u>=0&&u<=U_WH)?n[u]:"";
}
static void vib(){M5.Power.setVibration(true);delay(5);M5.Power.setVibration(false);}
static void beep(){M5.Speaker.tone(880,16);}
static void mark(){dirty=true;}
static void fmt(double x,char*o,size_t n){
  if(!isfinite(x)){snprintf(o,n,"ERR");return;}if(fabs(x)<1e-15)x=0;
  if(engMode&&x!=0){int e=(int)floor(log10(fabs(x))/3)*3;snprintf(o,n,"%.4gE%+d",x/pow(10,e),e);}
  else snprintf(o,n,"%.10g",x);
}
static void tapeAdd(const char*s){for(int i=7;i>0;i--)strncpy(tape[i],tape[i-1],47);strncpy(tape[0],s,47);tape[0][47]=0;if(tapeCount<8)tapeCount++;mark();}
static void tapeVal(const char*t,double x,Unit u){char b[48],n[24];fmt(x,n,sizeof n);snprintf(b,sizeof b,"%s %s %s",t,n,us(u));tapeAdd(b);}
static void setEntry(double x){fmt(x,entry,sizeof entry);entryLen=strlen(entry);entering=true;cur=x;mark();}
static double getEntry(){return strtod(entry,nullptr);}
static double curValue(){return entering?getEntry():cur;}
static void clearEntry(){
  strcpy(entry,"0");entryLen=1;entering=true;cur=0;op=0;stepTag="";
  stepOhm=stepFilt=stepVD=stepSlope=stepTrig=stepBake=stepPaint=stepAmort=stepConv=0;mark();
}
static void setReg(Value&v,double x,Unit u){v.x=x;v.u=u;v.set=true;reg.answer=v;setEntry(x);tapeVal("=",x,u);}
static bool need(const Value&v){if(!v.set){beep();return false;}return true;}
static void digit(int d){if(!entering||!strcmp(entry,"0")){entry[0]=char('0'+d);entry[1]=0;entryLen=1;entering=true;}else if(entryLen<38){entry[entryLen++]=char('0'+d);entry[entryLen]=0;}mark();}
static void dot(){if(!entering){strcpy(entry,"0.");entryLen=2;entering=true;}else if(!strchr(entry,'.')&&entryLen<37){entry[entryLen++]='.';entry[entryLen]=0;}mark();}
static void sign(){setEntry(-curValue());}
static void back(){if(!entering)return;if(entryLen>1)entry[--entryLen]=0;else{strcpy(entry,"0");entryLen=1;}mark();}
static void binary(char o){
  double x=curValue();
  if(op){double r=cur;if(op=='+')r+=x;else if(op=='-')r-=x;else if(op=='*')r*=x;else if(op=='/')r=fabs(x)>1e-15?cur/x:NAN;else if(op=='^')r=pow(cur,x);else if(op=='%')r=fmod(cur,x);cur=r;}
  else cur=x;op=o;entering=false;mark();
}
static void equals(){
  double x=curValue();
  if(op){double left=cur,r=left;
    if(op=='+')r+=x;else if(op=='-')r-=x;else if(op=='*')r*=x;else if(op=='/')r=fabs(x)>1e-15?left/x:NAN;else if(op=='^')r=pow(left,x);else if(op=='%')r=fmod(left,x);
    char b[48],a[16],z[16],q[16];fmt(left,a,16);fmt(x,z,16);fmt(r,q,16);
    snprintf(b,48,"%s %c %s=%s",a,op,z,q);tapeAdd(b);cur=r;op=0;setEntry(r);reg.answer.x=r;reg.answer.u=U_NONE;reg.answer.set=true;
  }else{cur=x;setEntry(x);tapeVal("=",x,U_NONE);}
}
static void initAwg(){for(int n=0;n<=40;n++){double d=5.0*pow(92.0,(36.0-n)/39.0);awgCm[n]=d*d;}}
static int awgFor(double amps,double lenFt,double volts,double pct){
  double K=useAl?K_AL:K_CU,t=volts*pct/100;for(int n=40;n>=0;n--)if(2*lenFt*amps*K/awgCm[n]<=t)return n;return 40;}
static double wireDrop(double amps,double lenFt,int awg){if(awg<0||awg>40)return NAN;return 2*lenFt*amps*(useAl?K_AL:K_CU)/awgCm[awg];}
static double Rpar(double a,double b){return fabs(a+b)>1e-15?a*b/(a+b):NAN;}

// === UNIT ENGINE ===
// SI scale for unit keys (2nd)
static void siPrefix(){
  double x=curValue();
  if(reg.v.set&&reg.v.u==U_V){setReg(reg.v,x*1e-3,U_V);return;}
  if(reg.a.set&&reg.a.u==U_A){setReg(reg.a,x*1e-3,U_A);return;}
  if(reg.r.set&&reg.r.u==U_OHM){setReg(reg.r,x*1e3,U_OHM);return;}
  if(reg.c.set&&reg.c.u==U_F){setReg(reg.c,x*1e-6,U_F);return;}
  if(reg.l.set&&reg.l.u==U_H){setReg(reg.l,x*1e-3,U_H);return;}
  // bare: cycle m ↔ base ↔ k by magnitude
  if(fabs(x)>=1e6)setEntry(x*1e-6);else if(fabs(x)>=1e3)setEntry(x*1e-3);else if(fabs(x)<1e-3&&x!=0)setEntry(x*1e6);else if(fabs(x)<1&&x!=0)setEntry(x*1e3);else setEntry(x*1e-3);
}

// Table-driven convert. stepConv cycles common pairs per page.
static void convert(){
  double x=curValue();
  stepConv=(stepConv%6)+1;
  if(page==0){ // ELEC: Al/Cu toggle first, then length/freq
    if(stepConv==1){useAl=!useAl;tapeAdd(useAl?"AL wire":"CU wire");stepTag=useAl?"Al":"Cu";return;}
    if(reg.hz.set){setReg(reg.answer,C_LIGHT/reg.hz.x,U_M);return;}
    if(reg.dist.set){setReg(reg.answer,reg.dist.x*12,U_IN);return;}
    setReg(reg.answer,x*12,U_IN);return;
  }
  if(page==1){ // MECH
    if(reg.rpm.set&&stepConv==1){setReg(reg.answer,reg.rpm.x/60,U_NONE);return;}
    if(reg.thrust.set){setReg(reg.answer,reg.thrust.x*0.00220462,U_LB);return;}
    if(reg.weight.set){setReg(reg.answer,reg.weight.x/1000,U_KG);return;}
    setEntry(x);return;
  }
  if(page==2){ // BUILD length
    if(reg.rise.set&&reg.run.set&&stepConv<=2){setReg(reg.answer,reg.rise.x/reg.run.x,U_NONE);return;}
    if(stepConv==1){setReg(reg.answer,x*12,U_IN);return;}
    if(stepConv==2){setReg(reg.answer,x/12,U_FT);return;}
    if(stepConv==3){setReg(reg.answer,x*0.3048,U_M);return;}
    setReg(reg.answer,x/0.3048,U_FT);return;
  }
  if(page==3){ // KITCHEN volume + temp chain
    if(reg.temp.set&&reg.temp.u==U_C){setReg(reg.temp,reg.temp.x*9/5+32,U_FAH);return;}
    if(reg.temp.set&&reg.temp.u==U_FAH){setReg(reg.temp,(reg.temp.x-32)*5/9,U_C);return;}
    // volume cycle: cup→tbsp→tsp→mL→L→gal→cup
    static const double f[]={1,16,48,236.588,0.236588,0.0157725}; // from cup
    static Unit tu[]={U_CUP,U_TBSP,U_TSP,U_ML,U_LITER,U_GAL};
    int i=(stepConv-1)%6;
    double cups=x; // assume entry is cups if no unit context
    if(reg.answer.u==U_TBSP)cups=x/16;else if(reg.answer.u==U_TSP)cups=x/48;
    else if(reg.answer.u==U_ML)cups=x/236.588;else if(reg.answer.u==U_LITER)cups=x/0.236588;
    else if(reg.answer.u==U_GAL)cups=x/0.0157725;
    setReg(reg.answer,cups*f[i],tu[i]);return;
  }
  if(page==4){if(reg.price.set&&reg.total.set){setReg(reg.answer,reg.price.x*reg.total.x,U_DOLLAR);return;}}
  setReg(reg.answer,x,U_NONE);
}

// === FRACTIONS ===
// TEST: 0.375→3/8  π→355/113
static void toFraction(){
  double x=curValue(),sgn=x<0?-1:1;x=fabs(x);
  if(!isfinite(x)||x>1e9){beep();return;}
  long whole=(long)floor(x);double frac=x-whole;
  if(frac<1e-12){char b[48];snprintf(b,48,"%ld/1",(long)(sgn*whole));tapeAdd(b);setEntry(sgn*whole);return;}
  const int MAX=24;const long MAXDEN=10000;
  double aa[MAX];int n=0;double r=frac;
  for(;n<MAX;n++){aa[n]=floor(r);double d=r-aa[n];if(d<1e-15){n++;break;}r=1.0/d;}
  long h0=1,h1=(long)aa[0],k0=0,k1=1,bn=h1,bd=k1;
  for(int i=1;i<n;i++){long h2=(long)aa[i]*h1+h0,k2=(long)aa[i]*k1+k0;if(k2>MAXDEN)break;h0=h1;h1=h2;k0=k1;k1=k2;bn=h1;bd=k1;}
  long num=whole*bd+bn;if(sgn<0)num=-num;
  char b[48];snprintf(b,48,"%ld/%ld",num,bd);tapeAdd(b);setEntry((double)num/(double)bd);
}

// === ELEC ===
// TEST: 120V 15A → R=8Ω P=1800W
static void ohmPwr(){
  stepOhm=(stepOhm%4)+1;stepTag="Ohm";
  double V=reg.v.x,I=reg.a.x,R=reg.r.x,P=reg.p.x;
  if(stepOhm==1){if(reg.v.set&&reg.a.set)setReg(reg.r,V/I,U_OHM);else if(reg.v.set&&reg.r.set)setReg(reg.a,V/R,U_A);else if(reg.a.set&&reg.r.set)setReg(reg.v,I*R,U_V);else if(reg.v.set&&reg.p.set)setReg(reg.a,P/V,U_A);else{beep();stepOhm=0;}}
  else if(stepOhm==2){if(reg.v.set&&reg.a.set)setReg(reg.p,V*I,U_W);else if(reg.v.set&&reg.r.set)setReg(reg.p,V*V/R,U_W);else if(reg.a.set&&reg.r.set)setReg(reg.p,I*I*R,U_W);else{beep();stepOhm=0;}}
  else if(stepOhm==3){if(reg.p.set&&reg.v.set)setReg(reg.r,V*V/P,U_OHM);else if(reg.p.set&&reg.a.set)setReg(reg.r,P/(I*I),U_OHM);else{beep();stepOhm=0;}}
  else{if(reg.p.set&&reg.r.set)setReg(reg.v,sqrt(P*R),U_V);else if(reg.p.set&&reg.v.set)setReg(reg.a,P/V,U_A);else{beep();stepOhm=0;}}
}
// TEST: R=100 C=1µF → fc≈1591.5Hz
static void filt(){
  stepFilt=(stepFilt%3)+1;stepTag="FILT";
  if(!need(reg.r)||!need(reg.c)){stepFilt=0;return;}
  double R=reg.r.x,C=reg.c.x;
  if(stepFilt==1){setReg(reg.hz,1/(2*PI_D*R*C),U_HZ);tapeVal("tau",R*C,U_NONE);}
  else if(stepFilt==2)setReg(reg.answer,R*C,U_NONE);
  else setReg(reg.answer,0.69314718056*R*C,U_NONE);
}
// TEST: 120V 15A 75ft 14AWG → 7.07V 5.89% req≈10
static void vdrop(){
  stepVD=(stepVD%4)+1;stepTag=useAl?"VDAl":"VDCu";
  if(!need(reg.v)||!need(reg.a)||!need(reg.dist)){stepVD=0;return;}
  int awg=12;if(reg.answer.set&&reg.answer.u==U_AWG)awg=(int)reg.answer.x;
  if(stepVD==1)setReg(reg.answer,wireDrop(reg.a.x,reg.dist.x,awg),U_V);
  else if(stepVD==2){double d=wireDrop(reg.a.x,reg.dist.x,awg);setReg(reg.answer,100*d/reg.v.x,U_PCT);}
  else if(stepVD==3){int n=awgFor(reg.a.x,reg.dist.x,reg.v.x,3);setReg(reg.answer,(double)n,U_AWG);tapeVal("DROP",wireDrop(reg.a.x,reg.dist.x,n),U_V);}
  else{int n=awgFor(reg.a.x,reg.dist.x,reg.v.x,1);setReg(reg.answer,(double)n,U_AWG);}
}
static void awgKey(){
  if(need(reg.a)&&need(reg.dist)&&need(reg.v)){int n=awgFor(reg.a.x,reg.dist.x,reg.v.x,3);setReg(reg.answer,(double)n,U_AWG);tapeVal("DROP",wireDrop(reg.a.x,reg.dist.x,n),U_V);}
  else{int n=(int)round(curValue());if(n>=0&&n<=40){setReg(reg.answer,(double)n,U_AWG);tapeVal("cmil",awgCm[n],U_CMIL);}}
}
static void cmilShow(){int n=(int)round(curValue());if(n>=0&&n<=40){setReg(reg.answer,awgCm[n],U_CMIL);tapeVal("AWG",n,U_AWG);}else beep();}
// Accumulator series/parallel: stamp R, then answer holds running total
static void seriesR(){if(need(reg.r)){if(reg.answer.set&&reg.answer.u==U_OHM)setReg(reg.answer,reg.r.x+reg.answer.x,U_OHM);else setReg(reg.answer,reg.r.x,U_OHM);}else if(need(reg.v)&&need(reg.a))setReg(reg.r,reg.v.x/reg.a.x,U_OHM);}
static void parallelR(){if(need(reg.r)){if(reg.answer.set&&reg.answer.u==U_OHM)setReg(reg.answer,Rpar(reg.r.x,reg.answer.x),U_OHM);else setReg(reg.answer,reg.r.x,U_OHM);}}
static void rc(){if(!need(reg.r)||!need(reg.c))return;setReg(reg.answer,reg.r.x*reg.c.x,U_NONE);tapeVal("fc",1/(2*PI_D*reg.r.x*reg.c.x),U_HZ);}
static void rlc(){if(!need(reg.l)||!need(reg.c))return;setReg(reg.hz,1/(2*PI_D*sqrt(reg.l.x*reg.c.x)),U_HZ);}
static void led(){if(!need(reg.v)||!need(reg.r))return;double vf=reg.answer.set?reg.answer.x:2.0;setReg(reg.a,(reg.v.x-vf)/reg.r.x,U_A);}
static void react(){if(!need(reg.hz))return;double f=reg.hz.x;if(reg.l.set){setReg(reg.answer,2*PI_D*f*reg.l.x,U_OHM);tapeVal("XL",reg.answer.x,U_OHM);}else if(reg.c.set){setReg(reg.answer,1/(2*PI_D*f*reg.c.x),U_OHM);tapeVal("XC",reg.answer.x,U_OHM);}}
static void pf(){if(need(reg.p)&&need(reg.v)&&need(reg.a))setReg(reg.answer,reg.p.x/(reg.v.x*reg.a.x),U_NONE);}
static void seriesC(){if(need(reg.c)&&need(reg.answer))setReg(reg.answer,1.0/(1/reg.c.x+1/reg.answer.x),U_F);}
static void parallelC(){if(need(reg.c)&&need(reg.answer))setReg(reg.answer,reg.c.x+reg.answer.x,U_F);}
static void capEnergy(){if(need(reg.c)&&need(reg.v))setReg(reg.answer,0.5*reg.c.x*reg.v.x*reg.v.x,U_W);}

// === MECH ===
static void motor(){if(!need(reg.kv)||!need(reg.cells))return;setReg(reg.rpm,reg.kv.x*reg.cells.x*4.2,U_RPM);}
static void gear(){if(!need(reg.ratio))return;if(reg.rpm.set)setReg(reg.answer,reg.rpm.x/reg.ratio.x,U_RPM);else if(reg.torque.set)setReg(reg.answer,reg.torque.x*reg.ratio.x,U_NONE);}
static void prop(){if(!need(reg.kv)||!need(reg.cells)||!need(reg.diameter))return;double rpm=reg.kv.x*reg.cells.x*4.2;double pitch=reg.pitch.set?reg.pitch.x:reg.diameter.x;reg.rpm.x=rpm;reg.rpm.u=U_RPM;reg.rpm.set=true;setReg(reg.answer,rpm*pitch*60/63360.0,U_NONE);tapeVal("RPM",rpm,U_RPM);}
static void battery(){if(need(reg.capacity)&&need(reg.cells))setReg(reg.energy,reg.capacity.x*reg.cells.x*3.7/1000.0,U_WH);else if(need(reg.weight)&&need(reg.thrust))setReg(reg.answer,reg.weight.x/reg.thrust.x,U_NONE);}
static void batteryWh(){if(need(reg.capacity)&&need(reg.cells))setReg(reg.answer,reg.capacity.x*reg.cells.x*3.7/1000.0,U_WH);}
static void torquePower(){if(need(reg.torque)&&need(reg.rpm))setReg(reg.answer,2*PI_D*reg.rpm.x*reg.torque.x/60.0,U_W);}
static void speed(){if(!need(reg.rpm)||!need(reg.pitch))return;setReg(reg.answer,reg.rpm.x*reg.pitch.x*60/63360.0,U_NONE);}
static void force(){if(need(reg.weight))setReg(reg.answer,reg.weight.x*G_ACC,U_NONE);}
static void mass(){if(need(reg.weight))setReg(reg.answer,reg.weight.x/G_ACC,U_KG);}

// === BUILD ===
// TEST: rise=12 run=16 → slope=0.75 ∠≈36.87° 75%
static void slope(){
  stepSlope=(stepSlope%3)+1;stepTag="SLP";
  if(!need(reg.rise)||!need(reg.run)){stepSlope=0;return;}
  double s=reg.rise.x/reg.run.x;
  if(stepSlope==1)setReg(reg.answer,s,U_NONE);
  else if(stepSlope==2)setReg(reg.angle,atan2(reg.rise.x,reg.run.x)*180/PI_D,U_DEG);
  else setReg(reg.answer,100*s,U_PCT);
}
static void angleSolve(){if(!need(reg.rise)||!need(reg.run))return;setReg(reg.angle,atan2(reg.rise.x,reg.run.x)*180/PI_D,U_DEG);}
static void riseFromAngle(){if(!need(reg.run)||!need(reg.angle))return;setReg(reg.rise,reg.run.x*tan(reg.angle.x*PI_D/180),U_FT);}
static void roof(){if(!need(reg.rise)||!need(reg.run))return;setReg(reg.answer,hypot(reg.rise.x,reg.run.x),U_FT);tapeVal("pitch",atan2(reg.rise.x,reg.run.x)*180/PI_D,U_DEG);}
static void stairs(){if(!need(reg.rise)||!need(reg.run))return;setReg(reg.answer,hypot(reg.rise.x,reg.run.x),U_FT);tapeVal("risers",reg.dist.set?ceil(reg.dist.x/reg.rise.x):ceil(reg.rise.x/7.5),U_NONE);}
static void stairCount(){if(!need(reg.rise))return;setReg(reg.answer,floor(reg.rise.x/7.5),U_NONE);}
static void board(){if(need(reg.rise)&&need(reg.run)&&need(reg.dist))setReg(reg.answer,reg.rise.x*reg.run.x*reg.dist.x/12.0,U_FT);else if(need(reg.volume))setReg(reg.answer,reg.volume.x/12,U_FT);}
static void concrete(){if(!need(reg.volume))return;setReg(reg.answer,reg.volume.x/27.0*1.10,U_YD3);}
// TEST: perim=48 h=8 doors=1 wins=2 coats=2 → net=333 gal≈1.90
static void paint(){
  stepPaint=(stepPaint%2)+1;stepTag="PNT";
  if(stepPaint==1){
    if(!need(reg.rise)||!need(reg.run)){stepPaint=0;return;}
    double net=reg.rise.x*reg.run.x-((reg.dist.set?reg.dist.x:0)*21+(reg.angle.set?reg.angle.x:0)*15);if(net<0)net=0;
    setReg(reg.area,net,U_FT);tapeVal("NET",net,U_FT);
  }else{if(!need(reg.area)){stepPaint=0;return;}double coats=reg.total.set?reg.total.x:1,cov=reg.price.set?reg.price.x:350;double g=reg.area.x*coats/cov;setReg(reg.answer,g,U_GAL);tapeVal("BUY",ceil(g),U_GAL);}
}
static void rectArea(){if(need(reg.rise)&&need(reg.run))setReg(reg.area,reg.rise.x*reg.run.x,U_FT);}
static void circleArea(){double d=curValue();setReg(reg.answer,PI_D*d*d/4,U_FT);}
static void cylVol(){
  double d=reg.diameter.set?reg.diameter.x:(reg.rise.set?reg.rise.x:curValue());
  double h=reg.run.set?reg.run.x:(reg.dist.set?reg.dist.x:0);
  if(h==0){beep();return;}setReg(reg.volume,PI_D*(d*0.5)*(d*0.5)*h,U_FT);
}
static void pyth(){if(need(reg.rise)&&need(reg.run))setReg(reg.answer,hypot(reg.rise.x,reg.run.x),U_FT);}
static void materialWaste(){setReg(reg.answer,curValue()*1.10,U_NONE);}
static void panRound(){double d=curValue();setReg(reg.answer,PI_D*d*d/4,U_FT);}

// === KITCHEN ===
static void bake(){
  stepBake=(stepBake%4)+1;stepTag="BAKE";
  if(stepBake==1&&need(reg.flour))setReg(reg.answer,reg.total.set?reg.total.x:reg.flour.x,U_G);
  else if(stepBake==2&&need(reg.flour)&&need(reg.hydration))setReg(reg.answer,reg.flour.x*reg.hydration.x/100,U_G);
  else if(stepBake==3&&need(reg.flour)&&need(reg.total))setReg(reg.hydration,(reg.total.x-reg.flour.x)/reg.flour.x*100,U_PCT);
  else if(stepBake==4&&need(reg.total)&&need(reg.servings))setReg(reg.answer,reg.total.x/reg.servings.x,U_G);
  else{beep();stepBake=0;}
}
static void recipe(){if(!need(reg.total)||!need(reg.servings))return;setReg(reg.answer,reg.total.x/reg.servings.x,U_G);}
static void hydWater(){if(need(reg.flour)&&need(reg.hydration))setReg(reg.answer,reg.flour.x*reg.hydration.x/100,U_G);}
static void recipePct(){if(need(reg.total)&&need(reg.flour))setReg(reg.answer,reg.total.x/reg.flour.x*100,U_PCT);}
static void density(){if(need(reg.total)&&need(reg.servings))setReg(reg.answer,reg.total.x/reg.servings.x,U_G);}

// === MONEY ===
// TEST: 100000 @5% 360mo → PMT≈536.82
static void loan(){
  if(!need(reg.principal)||!need(reg.rate)||!need(reg.term))return;
  double P=reg.principal.x,rm=reg.rate.x/100/12,n=reg.term.x;
  double pay=fabs(rm)<1e-15?P/n:P*rm*pow(1+rm,n)/(pow(1+rm,n)-1);
  reg.payment.x=pay;reg.payment.u=U_DOLLAR;reg.payment.set=true;reg.interest.x=pay*n-P;reg.interest.u=U_DOLLAR;reg.interest.set=true;
  setEntry(pay);tapeVal("PMT",pay,U_DOLLAR);tapeVal("INT",reg.interest.x,U_DOLLAR);stepTag="LOAN";
}
static void payoff(){
  if(!need(reg.principal)||!need(reg.rate)||!need(reg.payment))return;
  double bal=reg.principal.x,rm=reg.rate.x/100/12,pay=reg.payment.x,ex=reg.extra.set?reg.extra.x:0;
  int n=0;while(bal>0.01&&n<1200){double i=bal*rm,pp=pay+ex-i;if(pp<=0){bal=NAN;break;}bal-=pp;n++;}
  reg.term.x=(double)n;reg.term.u=U_NONE;reg.term.set=true;setEntry(n);tapeVal("MO",n,U_NONE);
}
static void compound(){double P=need(reg.principal)?reg.principal.x:curValue(),rate=need(reg.rate)?reg.rate.x/100:0,n=need(reg.term)?reg.term.x:1;double A=P*pow(1+rate/12,n);reg.savings.x=A;reg.savings.u=U_DOLLAR;reg.savings.set=true;setEntry(A);tapeVal("FV",A,U_DOLLAR);}
static void interestOnly(){if(need(reg.principal)&&need(reg.rate))setReg(reg.answer,reg.principal.x*reg.rate.x/100/12,U_DOLLAR);}
static void payMonths(){if(need(reg.principal)&&need(reg.payment)&&need(reg.rate)){double r=reg.rate.x/100/12;double n=r>0?-log(1-r*reg.principal.x/reg.payment.x)/log(1+r):reg.principal.x/reg.payment.x;setReg(reg.answer,n,U_NONE);}}
static void present(){if(need(reg.payment)&&need(reg.term)&&need(reg.rate)){double r=reg.rate.x/100/12;double pv=r?reg.payment.x*(1-pow(1+r,-reg.term.x))/r:reg.payment.x*reg.term.x;setReg(reg.answer,pv,U_DOLLAR);}}
static void cashflow(){if(need(reg.price)&&need(reg.total))setReg(reg.answer,reg.price.x*reg.total.x,U_DOLLAR);}
static void amortStep(){
  if(!need(reg.principal)||!need(reg.rate)||!need(reg.payment))return;
  stepAmort++;stepTag="AM";
  double bal=reg.principal.x,rm=reg.rate.x/100/12,pay=reg.payment.x,iamt=0;
  for(int i=0;i<stepAmort&&bal>0;i++){iamt=bal*rm;bal-=(pay-iamt);if(bal<0)bal=0;}
  setReg(reg.answer,bal,U_DOLLAR);tapeVal("BAL",bal,U_DOLLAR);tapeVal("I",iamt,U_DOLLAR);tapeVal("MO",stepAmort,U_NONE);
}

// === MATH / STAT ===
static void trig(Act a){
  double x=curValue(),r=0;
  if(a==A_SIN)r=sin(x*PI_D/180);else if(a==A_COS)r=cos(x*PI_D/180);else if(a==A_TAN)r=tan(x*PI_D/180);
  else if(a==A_ASIN)r=asin(x)*180/PI_D;else if(a==A_ACOS)r=acos(x)*180/PI_D;else r=atan(x)*180/PI_D;
  setEntry(r);reg.answer.x=r;reg.answer.u=U_NONE;reg.answer.set=true;tapeVal("TRIG",r,U_NONE);
}
static void hyper(Act a){double x=curValue();setEntry(a==A_SINH?sinh(x):a==A_COSH?cosh(x):tanh(x));}
static void statAdd(){double x=curValue();if(stN==0){stMin=stMax=x;stSum=0;}if(x<stMin)stMin=x;if(x>stMax)stMax=x;stSum+=x;stN++;tapeVal("n",stN,U_NONE);setEntry(x);stepTag="STAT";}
static void statMinMax(){if(stN==0){beep();return;}char b[48];snprintf(b,48,"min%.4g max%.4g",stMin,stMax);tapeAdd(b);setEntry(stMin);tapeVal("Σ",stSum,U_NONE);tapeVal("μ",stSum/stN,U_NONE);}
static void statClr(){stN=0;stMin=stMax=stSum=0;tapeAdd("STAT CLR");}

// === DISPATCH ===
static void doAct(Act a){
  double x=curValue();
  switch(a){
  case A_V:setReg(reg.v,x,U_V);break;case A_A:setReg(reg.a,x,U_A);break;case A_R:setReg(reg.r,x,U_OHM);break;case A_P:setReg(reg.p,x,U_W);break;
  case A_C:setReg(reg.c,x,U_F);break;case A_L:setReg(reg.l,x,U_H);break;case A_HZ:setReg(reg.hz,x,U_HZ);break;case A_DIST:setReg(reg.dist,x,U_FT);break;
  case A_RISE:setReg(reg.rise,x,U_FT);break;case A_RUN:setReg(reg.run,x,U_FT);break;case A_ANGLE:setReg(reg.angle,x,U_DEG);break;
  case A_AREA:setReg(reg.area,x,U_FT);break;case A_VOLUME:setReg(reg.volume,x,U_FT);break;
  case A_KV:setReg(reg.kv,x,U_KV);break;case A_CELLS:setReg(reg.cells,x,U_CELL);break;case A_RPM:setReg(reg.rpm,x,U_RPM);break;
  case A_THRUST:setReg(reg.thrust,x,U_G);break;case A_WEIGHT:setReg(reg.weight,x,U_G);break;case A_RATIO:setReg(reg.ratio,x,U_NONE);break;
  case A_PITCH:setReg(reg.pitch,x,U_IN);break;case A_DIAM:setReg(reg.diameter,x,U_IN);break;case A_TORQUE:setReg(reg.torque,x,U_NONE);break;
  case A_ENERGY:setReg(reg.energy,x,U_WH);break;case A_CAPACITY:setReg(reg.capacity,x,U_NONE);break;
  case A_FLOUR:setReg(reg.flour,x,U_G);break;case A_TOTAL:setReg(reg.total,x,U_G);break;case A_HYDR:setReg(reg.hydration,x,U_PCT);break;
  case A_SERV:setReg(reg.servings,x,U_NONE);break;case A_TEMP:setReg(reg.temp,x,U_C);break;case A_TEMPF:setReg(reg.temp,x,U_FAH);break;
  case A_PRICE:setReg(reg.price,x,U_DOLLAR);break;case A_PRINC:setReg(reg.principal,x,U_DOLLAR);break;case A_RATE:setReg(reg.rate,x,U_PCT);break;
  case A_TERM:setReg(reg.term,x,U_NONE);break;case A_EXTRA:setReg(reg.extra,x,U_DOLLAR);break;
  case A_ADD:binary('+');break;case A_SUB:binary('-');break;case A_MUL:binary('*');break;case A_DIV:binary('/');break;
  case A_EQ:equals();break;case A_DOT:dot();break;case A_SIGN:sign();break;case A_BS:back();break;case A_CLR:clearEntry();break;
  case A_MC:memory=0;tapeAdd("MC");break;case A_MR:setEntry(memory);break;case A_MS:memory=x;tapeVal("MS",x,U_NONE);break;case A_MPLUS:memory+=x;tapeVal("M+",x,U_NONE);break;
  case A_OHMPWR:ohmPwr();break;case A_FILT:filt();break;case A_VDROP:vdrop();break;case A_AWG:awgKey();break;case A_CMIL:cmilShow();break;
  case A_SERIES:case A_SERIESR:seriesR();break;case A_PARALLEL:case A_PARALLELR:parallelR();break;
  case A_RC:rc();break;case A_RLC:rlc();break;case A_LED:led();break;
  case A_DB:setReg(reg.answer,20*log10(fabs(x)+1e-30),U_NONE);break;
  case A_WAVELEN:if(reg.hz.set)setReg(reg.answer,C_LIGHT/reg.hz.x,U_M);else if(x>0)setReg(reg.answer,C_LIGHT/x,U_M);break;
  case A_GEAR:gear();break;case A_PROP:prop();break;case A_BATTERY:battery();break;case A_MOTOR:motor();break;
  case A_SLOPE:slope();break;case A_ANGLE_SOLVE:angleSolve();break;case A_RISE_FROM_ANGLE:riseFromAngle();break;
  case A_ROOF:roof();break;case A_STAIRS:stairs();break;case A_STAIRCOUNT:stairCount();break;case A_BOARD:board();break;
  case A_PAINT:paint();break;case A_CONCRETE:concrete();break;
  case A_TRIG:stepTrig=(stepTrig%3)+1;stepTag="TRI";trig(stepTrig==1?A_SIN:stepTrig==2?A_COS:A_TAN);break;
  case A_BAKE:bake();break;case A_RECIPE:recipe();break;case A_DENSITY:density();break;case A_CONVERT:convert();break;
  case A_LOAN:loan();break;case A_AMORT:amortStep();break;case A_COMPOUND:compound();break;case A_PAYOFF:payoff();break;
  case A_PERCENT:setEntry(x/100);reg.answer.x=x/100;reg.answer.u=U_PCT;reg.answer.set=true;break;
  case A_SQRT:setEntry(sqrt(x));break;case A_SQUARE:setEntry(x*x);break;case A_RECIP:setEntry(1/x);break;
  case A_LOG:setEntry(log10(x));break;case A_LN:setEntry(log(x));break;case A_EXP:setEntry(exp(x));break;case A_POW10:setEntry(pow(10,x));break;
  case A_SIN:case A_COS:case A_TAN:case A_ASIN:case A_ACOS:case A_ATAN:trig(a);break;
  case A_FRAC:toFraction();break;case A_ROUND:setEntry(round(x*100)/100);break;case A_CEIL:setEntry(ceil(x));break;case A_FLOOR:setEntry(floor(x));break;
  case A_STAT:statAdd();break;case A_MINMAX:statMinMax();break;case A_STATCLR:statClr();break;
  case A_PI:setEntry(PI_D);reg.answer.x=PI_D;reg.answer.u=U_NONE;reg.answer.set=true;break;case A_E:setEntry(2.718281828459045);break;
  case A_ANS:if(reg.answer.set)setEntry(reg.answer.x);else setEntry(memory);break;case A_ABS:setEntry(fabs(x));break;
  case A_POW:binary('^');break;case A_MOD:binary('%');break;
  case A_EE:if(!entering){strcpy(entry,"1e");entryLen=2;entering=true;}else if(entryLen<35){entry[entryLen++]='e';entry[entryLen]=0;}mark();break;
  case A_PERCENTDELTA:if(fabs(cur)<1e-15)setEntry(NAN);else setEntry((x-cur)/fabs(cur)*100);break;
  case A_CIRC:setReg(reg.answer,PI_D*x*x/4,U_FT);tapeVal("C",PI_D*x,U_FT);break;
  case A_REACT:react();break;case A_PF:pf();break;case A_SERIESC:seriesC();break;case A_PARALLELC:parallelC();break;
  case A_CAPENERGY:capEnergy();break;case A_BATTERYWH:batteryWh();break;case A_TORQUEPOWER:torquePower();break;case A_SPEED:speed();break;
  case A_FORCE:force();break;case A_MASS:mass();break;case A_RECTAREA:rectArea();break;case A_CIRCLEAREA:circleArea();break;
  case A_CYLVOL:cylVol();break;case A_PYTH:pyth();break;case A_MATERIALWASTE:materialWaste();break;case A_PANROUND:panRound();break;
  case A_HYDRWATER:hydWater();break;case A_RECIPEPCT:recipePct();break;case A_FV:compound();break;case A_INTEREST:interestOnly();break;
  case A_PAYMONTHS:payMonths();break;case A_PRESENT:present();break;case A_CASHFLOW:cashflow();break;
  case A_DEG2RAD:setEntry(x*PI_D/180);break;case A_RAD2DEG:setEntry(x*180/PI_D);break;
  case A_SINH:hyper(A_SINH);break;case A_COSH:hyper(A_COSH);break;case A_TANH:hyper(A_TANH);break;
  case A_SIGFIG:{if(x==0)setEntry(0);else{double p=pow(10,3-floor(log10(fabs(x))));setEntry(round(x*p)/p);}}break;
  case A_SIPREFIX:siPrefix();break;case A_ENG:engMode=!engMode;tapeAdd(engMode?"ENG ON":"ENG OFF");setEntry(curValue());break;
  default:break;
  }
  if(shift&&a!=A_NONE&&a!=A_ADD&&a!=A_SUB&&a!=A_MUL&&a!=A_DIV&&a!=A_EQ)shift=false;
  mark();
}

// === UI ===
static void drawKey(int x,int y,int w,int h,const Key&k,bool sh){
  // skeuomorphic raised key: face + light top edge + dark bottom edge
  uint16_t face=sh?0x580B:0x19C7;      // purple when 2nd, teal-dark otherwise
  uint16_t hi=sh?0xA81F:0x3A8A;        // highlight
  uint16_t lo=sh?0x3006:0x0C63;        // shadow
  M5.Display.fillRoundRect(x,y,w,h,5,face);
  M5.Display.drawFastHLine(x+2,y+1,w-4,hi);
  M5.Display.drawFastHLine(x+2,y+h-2,w-4,lo);
  M5.Display.drawFastVLine(x+1,y+2,h-4,hi);
  M5.Display.drawFastVLine(x+w-2,y+2,h-4,lo);
  const char*s=sh&&k.s[0]?k.s:k.p;
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(sh?0xAF27:0xFFFF); // lime label when 2nd
  int tw=M5.Display.textWidth(s);M5.Display.setCursor(x+(w-tw)/2,y+6);M5.Display.print(s);
  if(k.s[0]&&!sh){M5.Display.setTextColor(0xAF27);M5.Display.setCursor(x+3,y+h-11);M5.Display.print("2");}
}
static void drawRegFlags(){
  M5.Display.setTextSize(1);
  int x=188;
  const char* labs[]={"V","A","R","P","C","L","f","d"};
  Value* vs[]={&reg.v,&reg.a,&reg.r,&reg.p,&reg.c,&reg.l,&reg.hz,&reg.dist};
  for(int i=0;i<8;i++){
    if(!vs[i]->set)continue;
    M5.Display.fillRoundRect(x,20,10,12,2,0x038E);
    M5.Display.setTextColor(0xFFFF);M5.Display.setCursor(x+2,22);M5.Display.print(labs[i]);
    x+=12;
  }
}
static void draw(){
  if(!dirty)return;dirty=false;
  M5.Display.fillScreen(0x08C3);
  M5.Display.fillRect(0,0,W,TOP,accents[page]);
  M5.Display.setTextColor(0xFFFF);M5.Display.setTextSize(1);
  M5.Display.setCursor(4,3);
  M5.Display.printf("%s",pageNames[page]);
  if(shift){M5.Display.fillRoundRect(70,2,28,12,2,0x580B);M5.Display.setCursor(74,4);M5.Display.print("2ND");}
  if(engMode){M5.Display.setCursor(104,3);M5.Display.print("ENG");}
  if(stepTag[0]){M5.Display.setCursor(140,3);M5.Display.print(stepTag);}
  if(useAl&&page==0){M5.Display.setCursor(170,3);M5.Display.print("Al");}
  char n[32];fmt(curValue(),n,sizeof n);
  M5.Display.setTextSize(2);M5.Display.setCursor(4,14);M5.Display.print(n);
  drawRegFlags();
  M5.Display.fillRect(0,TAPE_Y,W,TAPE_H,0x0C63);
  M5.Display.setTextSize(1);M5.Display.setTextColor(0x9CD3);
  for(int i=0;i<2&&i<tapeCount;i++){M5.Display.setCursor(4,TAPE_Y+1+i*8);M5.Display.print(tape[i]);}
  for(int i=0;i<12;i++){int row=i/6,col=i%6;drawKey(col*53+1,FUNC_Y+row*34,51,32,pages[page][i],shift);}
  const char*nums[16]={"7","8","9","/","4","5","6","*","1","2","3","-","0",".","=","+"};
  const char*secs[16]={"pi","e","%","^","BS","+/-","ABS","mod","1/x","ANS","EE","d%","CLR","MR","MS","MC"};
  for(int i=0;i<16;i++){
    int row=i/4,col=i%4,x=2+col*79,y=NUM_Y+row*22;
    bool isOp=(i==3||i==7||i==11||i==15||i==14);
    uint16_t face=shift?0x580B:(isOp?0x038E:0x19C7);
    uint16_t hi=shift?0xA81F:(isOp?0x04D1:0x3A8A);
    uint16_t lo=shift?0x3006:(isOp?0x01C6:0x0C63);
    M5.Display.fillRoundRect(x,y,76,20,4,face);
    M5.Display.drawFastHLine(x+2,y+1,72,hi);
    M5.Display.drawFastHLine(x+2,y+18,72,lo);
    M5.Display.setTextColor(shift?0xAF27:0xFFFF);M5.Display.setTextSize(1);
    const char*lab=shift?secs[i]:nums[i];
    int tw=M5.Display.textWidth(lab);M5.Display.setCursor(x+(76-tw)/2,y+4);M5.Display.print(lab);
  }
  M5.Display.fillRect(0,BAR_Y,W,H-BAR_Y,0x0C63);
  M5.Display.drawFastHLine(0,BAR_Y,W,0x038E);
  M5.Display.setTextColor(0xFFFF);M5.Display.setTextSize(1);
  M5.Display.fillRoundRect(8,BAR_Y+4,60,16,4,0x19C7);
  M5.Display.drawFastHLine(10,BAR_Y+5,56,0x3A8A);
  M5.Display.setCursor(22,BAR_Y+7);M5.Display.print("< A");
  M5.Display.fillRoundRect(130,BAR_Y+4,60,16,4,shift?0x580B:0x19C7);
  M5.Display.drawFastHLine(132,BAR_Y+5,56,shift?0xA81F:0x3A8A);
  M5.Display.setTextColor(shift?0xAF27:0xFFFF);
  M5.Display.setCursor(140,BAR_Y+7);M5.Display.print(shift?"2ND*":"B 2ND");
  M5.Display.setTextColor(0xFFFF);
  M5.Display.fillRoundRect(252,BAR_Y+4,60,16,4,0x19C7);
  M5.Display.drawFastHLine(254,BAR_Y+5,56,0x3A8A);
  M5.Display.setCursor(266,BAR_Y+7);M5.Display.print("C >");
}
static void touch(){
  auto t=M5.Touch.getDetail();if(!t.isPressed())return;
  if(millis()-lastTouch<110)return;lastTouch=millis();
  int x=t.x,y=t.y;vib();
  if(y>=BAR_Y){
    if(x<106){page=(page+5)%6;clearEntry();shift=false;}
    else if(x>212){page=(page+1)%6;clearEntry();shift=false;}
    else shift=!shift;
    mark();return;
  }
  if(y>=FUNC_Y&&y<FUNC_Y+68){
    int col=x/53,row=(y-FUNC_Y)/34,i=row*6+col;
    if(i>=0&&i<12){Key&k=pages[page][i];doAct(shift&&k.b!=A_NONE?k.b:k.a);}
    return;
  }
  if(y>=NUM_Y){
    int row=(y-NUM_Y)/22,col=x/79;
    if(row<4&&col<4){
      int i=row*4+col;
      if(shift){
        switch(i){
          case 0:doAct(A_PI);break;case 1:doAct(A_E);break;case 2:doAct(A_PERCENT);break;case 3:doAct(A_POW);break;
          case 4:doAct(A_BS);break;case 5:doAct(A_SIGN);break;case 6:doAct(A_ABS);break;case 7:doAct(A_MOD);break;
          case 8:doAct(A_RECIP);break;case 9:doAct(A_ANS);break;case 10:doAct(A_EE);break;case 11:doAct(A_PERCENTDELTA);break;
          case 12:doAct(A_CLR);break;case 13:doAct(A_MR);break;case 14:doAct(A_MS);break;case 15:doAct(A_MC);break;
        }
        shift=false;
      }else switch(i){
        case 0:digit(7);break;case 1:digit(8);break;case 2:digit(9);break;case 3:doAct(A_DIV);break;
        case 4:digit(4);break;case 5:digit(5);break;case 6:digit(6);break;case 7:doAct(A_MUL);break;
        case 8:digit(1);break;case 9:digit(2);break;case 10:digit(3);break;case 11:doAct(A_SUB);break;
        case 12:digit(0);break;case 13:dot();break;case 14:equals();break;case 15:doAct(A_ADD);break;
      }
    }
  }
}

static void splash(){
  M5.Display.fillScreen(0x08C3);
  M5.Display.fillRoundRect(20,16,280,200,14,0x4A69);
  M5.Display.fillRoundRect(26,22,268,188,12,0x19C7);
  // raw RGB565 logo centered
  int lx=(W-(int)logo_w)/2, ly=36;
  M5.Display.pushImage(lx, ly, logo_w, logo_h, logo_rgb565);
  M5.Display.setTextColor(0xAF27);M5.Display.setTextSize(2);
  const char* title="MANTISCALC";
  int tw=M5.Display.textWidth(title);
  M5.Display.setCursor((W-tw)/2, ly+(int)logo_h+8);M5.Display.print(title);
  M5.Display.setTextSize(1);M5.Display.setTextColor(0xFFFF);
  const char* sub="THE ULTIMATE CALCULATOR";
  tw=M5.Display.textWidth(sub);
  M5.Display.setCursor((W-tw)/2, ly+(int)logo_h+30);M5.Display.print(sub);
  M5.Speaker.tone(660,40);delay(30);M5.Speaker.tone(880,60);
  delay(1600);
}
void setup(){
  auto cfg=M5.config();M5.begin(cfg);
  M5.Display.setRotation(1);M5.Display.setTextWrap(false);M5.Speaker.setVolume(28);
  initAwg();splash();dirty=true;draw();
}
void loop(){
  M5.update();
  if(M5.BtnA.wasPressed()){page=(page+5)%6;clearEntry();shift=false;vib();mark();}
  if(M5.BtnC.wasPressed()){page=(page+1)%6;clearEntry();shift=false;vib();mark();}
  if(M5.BtnB.wasPressed()){shift=!shift;vib();mark();}
  touch();
  static unsigned long last=0;
  if(dirty||millis()-last>200){draw();last=millis();}
  delay(4);
}
