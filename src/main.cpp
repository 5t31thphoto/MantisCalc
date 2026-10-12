#include <M5Unified.h>
#include <M5GFX.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "logo_rgb565.h"

// MantisCalculator v1.7 - full multitool. Dense. Agent-maintained.
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
static double cur=0,disp=0,memory=0,awgCm[41];
static bool entering=false,shift=false,dirty=true,engMode=false,useAl=false;
static char op=0,entry[40]="0";static int entryLen=1,page=0,tapeCount=0;
static char tape[8][48];
static uint8_t stepOhm=0,stepFilt=0,stepVD=0,stepSlope=0,stepTrig=0,stepBake=0,stepPaint=0,stepAmort=0,stepConv=0;
static double pPerim=0,pHt=0,pDoors=0,pWins=0,pCoats=1,pCov=350;
static double wV=0,wA=0,wR=0,wC=0,wDist=0,wAwg=12,wRise=0,wRun=0;
static double wFlour=0,wHydr=0,wP=0,wRate=0,wN=0,wKv=0,wCells=0,wDiam=0,wPitch=0,wMah=0;
static unsigned long lastTouch=0;
static bool helpMode=false;static int helpScroll=0;
// On-device guide (2ND + tap header). ASCII only.
static const char* helpText[6][14]={
 // MATH
 {"MATH - scientific","Normal calculator first","x^2 sqrt 1/x LOG LN","SIN COS TAN degrees","FRAC best fraction a/b","ENG engineering display","STAT add sample / min max","2ND num: pi e % ^ BS CLR","Memory: MR MS MC ANS EE","Enter number, press key","", "", "", ""},
 // ELEC
 {"ELEC - electrical","Press tool first to queue","OHM/PWR: V -> A -> R,P","FILT: R -> C -> fc,tau","VDROP: V A ft awg -> drop","  header shows step name","Unit keys still stamp","VOLTS AMPS OHMS etc","AWG: gauge or recommend","2ND: mV mA kOhm uF wave","Al/Cu: 2ND CONV toggle","Ex: 120 OHM/PWR 15 OHM/PWR","  -> 8 ohm 1800 W", ""},
 // MECH
 {"MECH/RF - motors","MOTOR: KV -> S -> RPM","PROP: KV S diam pitch mph","BAT: mAh -> S -> Wh","Enter value, press key","each time for next step","FREQ WAVE wavelength","RATIO GEAR helpers","", "", "", "", "", ""},
 // BUILD
 {"BUILD - trades","SLOPE: rise -> run -> %","ROOF: rise -> run -> len","BOARD: T W L -> board-ft","PAINT: perim ht doors","  wins coats cov -> gal","CONC: volume -> yd3","TRIG sin cos tan cycle","Ex paint: 48 PAINT 8 PAINT","1 PAINT 2 PAINT 2 PAINT","350 PAINT -> gal", "", "", ""},
 // KITCHEN
 {"KITCHEN - baking","BAKE: flour -> hydr% -> H2O","CONV: cup tbsp tsp mL L","TEMP stamps C / 2ND F","RECIPE scale per serving","Enter then press BAKE", "", "", "", "", "", "", "", ""},
 // MONEY
 {"MONEY - loans","PMT wizard:","  PV -> APR% -> months","  -> payment + interest","Ex: 100000 PMT","  5 PMT  360 PMT  PMT","  -> ~536.82","AMORT steps balance","PAYOFF months to clear","", "", "", "", ""}
};

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
  A_SIPREFIX,A_MV,A_MA,A_KOHM,A_UF,A_MH,A_ALCU,A_TEMPCONV,A_CMIL,A_ANGLE_SOLVE,A_RISE_FROM_ANGLE,A_ENG,A_STATCLR,A_SERIESR,A_PARALLELR
};
struct Key{const char* p;const char* s;Act a,b;};

static const char* pageNames[]={"MATH","ELEC","MECH/RF","BUILD","KITCHEN","MONEY"};
static const uint16_t accents[]={0xC700,0x038E,0x580B,0xAF27,0x0451,0x780F}; // MATH lime, ELEC teal, ...

// === KEYS: every secondary label matches its action ===
static Key pages[6][12]={
 {{"x^2","sqrt",A_SQUARE,A_SQRT},{"sqrt","x^2",A_SQRT,A_SQUARE},{"1/x","%",A_RECIP,A_PERCENT},{"LOG","10^x",A_LOG,A_POW10},{"LN","e^x",A_LN,A_EXP},{"e^x","LN",A_EXP,A_LN},
  {"SIN","asin",A_SIN,A_ASIN},{"COS","acos",A_COS,A_ACOS},{"TAN","atan",A_TAN,A_ATAN},{"WAVE","f",A_WAVELEN,A_HZ},{"FRAC","ENG",A_FRAC,A_ENG},{"STAT","minMx",A_STAT,A_MINMAX}},
 {{"VOLTS","mV",A_V,A_MV},{"AMPS","mA",A_A,A_MA},{"OHMS","kOhm",A_R,A_KOHM},{"WATTS","dB",A_P,A_DB},{"FARAD","uF",A_C,A_UF},{"Hz","wave",A_HZ,A_WAVELEN},
  {"DIST","PF",A_DIST,A_PF},{"OHM/PWR","XL/XC",A_OHMPWR,A_REACT},{"VDROP","Al/Cu",A_VDROP,A_ALCU},{"FILT","RLC",A_FILT,A_RLC},{"AWG","cmil",A_AWG,A_CMIL},{"HENRY","mH",A_L,A_MH}},
 {{"KV","RPM",A_KV,A_RPM},{"CELLS","xV",A_CELLS,A_CONVERT},{"RPM","rps",A_RPM,A_CONVERT},{"THRUST","lb",A_THRUST,A_CONVERT},{"WEIGHT","kg",A_WEIGHT,A_CONVERT},{"RATIO","gear",A_RATIO,A_GEAR},
  {"PROP","mph",A_PROP,A_SPEED},{"GEAR","Pwr",A_GEAR,A_TORQUEPOWER},{"FREQ","wave",A_HZ,A_WAVELEN},{"WAVE","ft",A_WAVELEN,A_CONVERT},{"BAT","Wh",A_BATTERY,A_BATTERYWH},{"MOTOR","torq",A_MOTOR,A_TORQUE}},
 {{"RISE","slope",A_RISE,A_SLOPE},{"RUN","ang",A_RUN,A_ANGLE_SOLVE},{"SLOPE","ang",A_SLOPE,A_ANGLE_SOLVE},{"ANGLE","rise",A_ANGLE,A_RISE_FROM_ANGLE},{"AREA","circ",A_AREA,A_CIRCLEAREA},{"VOL","cyl",A_VOLUME,A_CYLVOL},
  {"ROOF","hyp",A_ROOF,A_PYTH},{"STAIRS","#",A_STAIRS,A_STAIRCOUNT},{"BOARD","+10%",A_BOARD,A_MATERIALWASTE},{"PAINT","step",A_PAINT,A_RECTAREA},{"CONC","yd3",A_CONCRETE,A_CONVERT},{"TRIG","inv",A_TRIG,A_ASIN}},
 {{"FLOUR","%",A_FLOUR,A_RECIPEPCT},{"TOTAL","/serv",A_TOTAL,A_SERV},{"HYDR%","H2O",A_HYDR,A_HYDRWATER},{"SERV","dens",A_SERV,A_DENSITY},{"TEMP","F",A_TEMP,A_TEMPF},{"PRICE","x",A_PRICE,A_TOTAL},
  {"BAKE","next",A_BAKE,A_HYDRWATER},{"RECIPE","scale",A_RECIPE,A_RECIPEPCT},{"DENS","g/cup",A_DENSITY,A_CONVERT},{"CONV","C<>F",A_CONVERT,A_TEMPCONV},{"AREA","pan",A_AREA,A_PANROUND},{"VOL","pan",A_VOLUME,A_CYLVOL}},
 {{"PRINC","PV",A_PRINC,A_PRESENT},{"RATE%","I/mo",A_RATE,A_INTEREST},{"TERM","n mo",A_TERM,A_PAYMONTHS},{"PMT","solve",A_LOAN,A_AMORT},{"INT","total",A_INTEREST,A_LOAN},{"SAV","FV",A_COMPOUND,A_FV},
  {"PAYOFF","extra",A_PAYOFF,A_PAYMONTHS},{"AMORT","step",A_AMORT,A_CASHFLOW},{"%","d%",A_PERCENT,A_PERCENTDELTA},{"ROUND","sig",A_ROUND,A_SIGFIG},{"PRICE","x",A_PRICE,A_TOTAL},{"TOTAL","sum",A_TOTAL,A_CASHFLOW}}

};

// === CORE ===
static const char* us(Unit u){
  static const char* n[]={"","V","A","ohm","W","F","H","Hz","m","ft","in","yd","deg","rad","%","g","kg","lb","rpm","KV","S","C","F","cup","tbsp","tsp","floz","$","gal","L","AWG","cmil","yd3","mL","Wh"};
  return (u>=0&&u<=U_WH)?n[u]:"";
}
static void vib(){M5.Power.setVibration(180);delay(20);M5.Power.setVibration(0);}
static void beep(){M5.Speaker.tone(880,16);}
static void mark(){dirty=true;}
static void fmt(double x,char*o,size_t n){
  if(!isfinite(x)){snprintf(o,n,"ERR");return;}if(fabs(x)<1e-15)x=0;
  if(engMode&&x!=0){int e=(int)floor(log10(fabs(x))/3)*3;snprintf(o,n,"%.4gE%+d",x/pow(10,e),e);}
  else snprintf(o,n,"%.10g",x);
}
static void tapeAdd(const char*s){for(int i=7;i>0;i--)strncpy(tape[i],tape[i-1],47);strncpy(tape[0],s,47);tape[0][47]=0;if(tapeCount<8)tapeCount++;mark();}
static void tapeVal(const char*t,double x,Unit u){char b[48],n[24];fmt(x,n,sizeof n);snprintf(b,sizeof b,"%s %s %s",t,n,us(u));tapeAdd(b);}
static void setEntry(double x){disp=x;entering=false;mark();}
// typed(): user has keyed a number since the last consumer (wizards take it as their next input)
static bool typed(){return entering&&strcmp(entry,"0")!=0;}
static double getEntry(){return strtod(entry,nullptr);}
static double curValue(){return entering?getEntry():disp;}
static void clearEntry(){
  strcpy(entry,"0");entryLen=1;entering=true;cur=0;disp=0;op=0;stepTag="";helpMode=false;
  stepOhm=stepFilt=stepVD=stepSlope=stepTrig=stepBake=stepPaint=stepAmort=stepConv=0;mark();
}
static void setReg(Value&v,double x,Unit u){v.x=x;v.u=u;v.set=true;reg.answer=v;setEntry(x);tapeVal("=",x,u);}
static bool need(const Value&v){if(!v.set){beep();return false;}return true;}
static void digit(int d){if(!entering||!strcmp(entry,"0")||!strcmp(entry,"-0")){entry[0]=char('0'+d);entry[1]=0;entryLen=1;entering=true;}else if(entryLen<38){entry[entryLen++]=char('0'+d);entry[entryLen]=0;}mark();}
static void dot(){if(!entering){strcpy(entry,"0.");entryLen=2;entering=true;}else if(!strchr(entry,'.')&&!strchr(entry,'e')&&entryLen<37){entry[entryLen++]='.';entry[entryLen]=0;}mark();}
static void sign(){
  if(!entering){setEntry(-disp);return;}
  char*e=strchr(entry,'e');
  if(e){if(e[1]=='-')memmove(e+1,e+2,strlen(e+2)+1);else if(entryLen<36){memmove(e+2,e+1,strlen(e+1)+1);e[1]='-';}}
  else if(entry[0]=='-')memmove(entry,entry+1,strlen(entry));else if(entryLen<36){memmove(entry+1,entry,strlen(entry)+1);entry[0]='-';}
  entryLen=strlen(entry);mark();
}
static void back(){if(!entering)return;if(entryLen>1)entry[--entryLen]=0;else{strcpy(entry,"0");entryLen=1;}mark();}
static void binary(char o){
  double x=curValue();
  if(op){double r=cur;if(op=='+')r+=x;else if(op=='-')r-=x;else if(op=='*')r*=x;else if(op=='/')r=fabs(x)>1e-15?cur/x:NAN;else if(op=='^')r=pow(cur,x);else if(op=='%')r=fmod(cur,x);cur=r;}
  else cur=x;disp=cur;op=o;entering=false;mark();
}
static void equals(){
  double x=curValue();
  if(op){double left=cur,r=left;
    if(op=='+')r+=x;else if(op=='-')r-=x;else if(op=='*')r*=x;else if(op=='/')r=fabs(x)>1e-15?left/x:NAN;
    ;if((op=='+'||op=='-')&&fabs(r)<1e-12*fmax(fabs(left),fabs(x)))r=0;else if(op=='^')r=pow(left,x);else if(op=='%')r=fmod(left,x);
    char b[64],a[20],z[20],q[20];fmt(left,a,20);fmt(x,z,20);fmt(r,q,20);
    snprintf(b,64,"%s %c %s=%s",a,op,z,q);tapeAdd(b);cur=r;op=0;setEntry(r);reg.answer.x=r;reg.answer.u=U_NONE;reg.answer.set=true;
  }else{cur=x;setEntry(x);tapeVal("=",x,U_NONE);}
}
static void initAwg(){for(int n=0;n<=40;n++){double d=5.0*pow(92.0,(36.0-n)/39.0);awgCm[n]=d*d;}}
static int awgFor(double amps,double lenFt,double volts,double pct){
  double K=useAl?K_AL:K_CU,t=volts*pct/100;for(int n=40;n>=0;n--)if(2*lenFt*amps*K/awgCm[n]<=t)return n;return -1;}
static double awgD(int n){return n<0?NAN:(double)n;} // NAN -> tape shows ERR (needs > 1/0)
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
  if(page==1){ // ELEC: ft -> in
    if(!typed()&&reg.dist.set){setReg(reg.answer,reg.dist.x*12,U_IN);return;}
    setReg(reg.answer,x*12,U_IN);return;
  }
  if(page==2){ // MECH
    if(reg.rpm.set&&stepConv==1){setReg(reg.answer,reg.rpm.x/60,U_NONE);return;}
    if(reg.thrust.set){setReg(reg.answer,reg.thrust.x*0.00220462,U_LB);return;}
    if(reg.weight.set){setReg(reg.answer,reg.weight.x/1000,U_KG);return;}
    setEntry(x);return;
  }
  if(page==3){ // BUILD length
    if(reg.rise.set&&reg.run.set&&stepConv<=2){setReg(reg.answer,reg.rise.x/reg.run.x,U_NONE);return;}
    if(stepConv==1){setReg(reg.answer,x*12,U_IN);return;}
    if(stepConv==2){setReg(reg.answer,x/12,U_FT);return;}
    if(stepConv==3){setReg(reg.answer,x*0.3048,U_M);return;}
    setReg(reg.answer,x/0.3048,U_FT);return;
  }
  if(page==4){ // KITCHEN volume chain (temp lives on CONV 2nd)
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
  if(page==5){if(reg.price.set&&reg.total.set){setReg(reg.answer,reg.price.x*reg.total.x,U_DOLLAR);return;}}
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

static void ohmPwr(){
  if(stepOhm==0&&typed())stepOhm=1;
  if(stepOhm==0&&reg.v.set&&reg.a.set){setReg(reg.r,reg.v.x/reg.a.x,U_OHM);setReg(reg.p,reg.v.x*reg.a.x,U_W);stepTag="Ohm";return;}
  if(stepOhm==0&&reg.v.set&&reg.r.set){setReg(reg.a,reg.v.x/reg.r.x,U_A);setReg(reg.p,reg.v.x*reg.v.x/reg.r.x,U_W);stepTag="Ohm";return;}
  if(stepOhm==0&&reg.a.set&&reg.r.set){setReg(reg.v,reg.a.x*reg.r.x,U_V);setReg(reg.p,reg.a.x*reg.a.x*reg.r.x,U_W);stepTag="Ohm";return;}
  if(stepOhm==0){stepOhm=1;stepTag="V?";tapeAdd("enter V");return;}
  double x=curValue();
  if(stepOhm==1){wV=x;reg.v.x=x;reg.v.u=U_V;reg.v.set=true;stepTag="A?";tapeVal("V",x,U_V);stepOhm=2;setEntry(x);}
  else if(stepOhm==2){wA=x;reg.a.x=x;reg.a.u=U_A;reg.a.set=true;double R=wA!=0?wV/wA:NAN,P=wV*wA;
    reg.r.x=R;reg.r.u=U_OHM;reg.r.set=true;reg.p.x=P;reg.p.u=U_W;reg.p.set=true;
    setReg(reg.answer,R,U_OHM);tapeVal("P",P,U_W);stepTag="ohm";stepOhm=0;}
}

static void filt(){
  if(stepFilt==0&&typed())stepFilt=1;
  if(stepFilt==0&&reg.r.set&&reg.c.set){double R=reg.r.x,C=reg.c.x;setReg(reg.hz,1/(2*PI_D*R*C),U_HZ);tapeVal("tau",R*C,U_NONE);stepTag="fc";return;}
  if(stepFilt==0){stepFilt=1;stepTag="R?";tapeAdd("enter R");return;}
  double x=curValue();
  if(stepFilt==1){wR=x;reg.r.x=x;reg.r.u=U_OHM;reg.r.set=true;stepTag="C?";tapeVal("R",x,U_OHM);stepFilt=2;setEntry(x);}
  else if(stepFilt==2){wC=x;reg.c.x=x;reg.c.u=U_F;reg.c.set=true;
    setReg(reg.hz,1/(2*PI_D*wR*wC),U_HZ);tapeVal("tau",wR*wC,U_NONE);stepTag="fc";stepFilt=0;}
}

static void vdrop(){
  if(stepVD==0&&typed())stepVD=1;
  if(stepVD==0&&reg.v.set&&reg.a.set&&reg.dist.set){
    int awg=12;if(reg.answer.set&&reg.answer.u==U_AWG)awg=(int)reg.answer.x;
    double d=wireDrop(reg.a.x,reg.dist.x,awg);setReg(reg.answer,d,U_V);
    tapeVal("pct",100*d/reg.v.x,U_PCT);tapeVal("AWG3%",awgD(awgFor(reg.a.x,reg.dist.x,reg.v.x,3)),U_AWG);
    stepTag=useAl?"Al":"Cu";return;
  }
  if(stepVD==0){stepVD=1;stepTag="Vsrc?";tapeAdd("enter Vsrc");return;}
  double x=curValue();
  if(stepVD==1){wV=x;reg.v.x=x;reg.v.u=U_V;reg.v.set=true;stepTag="A?";tapeVal("V",x,U_V);stepVD=2;setEntry(x);}
  else if(stepVD==2){wA=x;reg.a.x=x;reg.a.u=U_A;reg.a.set=true;stepTag="ft?";tapeVal("A",x,U_A);stepVD=3;setEntry(x);}
  else if(stepVD==3){wDist=x;reg.dist.x=x;reg.dist.u=U_FT;reg.dist.set=true;stepTag="awg?";tapeVal("ft",x,U_FT);stepVD=4;setEntry(x);}
  else if(stepVD==4){wAwg=(x>=0&&x<=40)?x:12;double d=wireDrop(wA,wDist,(int)wAwg);
    setReg(reg.answer,d,U_V);tapeVal("pct",wV>0?100*d/wV:NAN,U_PCT);
    tapeVal("AWG3%",awgD(awgFor(wA,wDist,wV,3)),U_AWG);stepTag="drop";stepVD=0;}
}

static void awgKey(){
  if(reg.a.set&&reg.dist.set&&reg.v.set&&!typed()){int n=awgFor(reg.a.x,reg.dist.x,reg.v.x,3);if(n<0){setReg(reg.answer,NAN,U_AWG);tapeAdd("needs > 1/0");return;}setReg(reg.answer,(double)n,U_AWG);tapeVal("DROP",wireDrop(reg.a.x,reg.dist.x,n),U_V);}
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

static void motor(){
  static uint8_t st=0;if(st==0&&typed())st=1;
  if(st==0&&reg.kv.set&&reg.cells.set){setReg(reg.rpm,reg.kv.x*reg.cells.x*4.2,U_RPM);stepTag="RPM";return;}
  if(st==0){st=1;stepTag="KV?";tapeAdd("enter KV");return;}
  double x=curValue();
  if(st==1){wKv=x;reg.kv.x=x;reg.kv.u=U_KV;reg.kv.set=true;stepTag="S?";st=2;setEntry(x);}
  else if(st==2){wCells=x;reg.cells.x=x;reg.cells.u=U_CELL;reg.cells.set=true;setReg(reg.rpm,wKv*wCells*4.2,U_RPM);stepTag="RPM";st=0;}
}

static void gear(){if(!need(reg.ratio))return;if(reg.rpm.set)setReg(reg.answer,reg.rpm.x/reg.ratio.x,U_RPM);else if(reg.torque.set)setReg(reg.answer,reg.torque.x*reg.ratio.x,U_NONE);}
static void prop(){
  static uint8_t st=0;if(st==0&&typed())st=1;
  if(st==0&&reg.kv.set&&reg.cells.set&&reg.diameter.set){
    double rpm=reg.kv.x*reg.cells.x*4.2;double pitch=reg.pitch.set?reg.pitch.x:reg.diameter.x;
    reg.rpm.x=rpm;reg.rpm.u=U_RPM;reg.rpm.set=true;setReg(reg.answer,rpm*pitch*60/63360.0,U_NONE);tapeVal("RPM",rpm,U_RPM);return;
  }
  if(st==0){st=1;stepTag="KV?";tapeAdd("enter KV");return;}
  double x=curValue();
  if(st==1){wKv=x;reg.kv.x=x;reg.kv.u=U_KV;reg.kv.set=true;stepTag="S?";st=2;setEntry(x);}
  else if(st==2){wCells=x;reg.cells.x=x;reg.cells.u=U_CELL;reg.cells.set=true;stepTag="diam?";st=3;setEntry(x);}
  else if(st==3){wDiam=x;reg.diameter.x=x;reg.diameter.u=U_IN;reg.diameter.set=true;stepTag="pitch?";st=4;setEntry(x);}
  else if(st==4){wPitch=x>0?x:wDiam;double rpm=wKv*wCells*4.2;reg.rpm.x=rpm;reg.rpm.set=true;
    setReg(reg.answer,rpm*wPitch*60/63360.0,U_NONE);tapeVal("RPM",rpm,U_RPM);stepTag="mph";st=0;}
}

static void battery(){
  static uint8_t st=0;if(st==0&&typed())st=1;
  if(st==0&&reg.capacity.set&&reg.cells.set){setReg(reg.energy,reg.capacity.x*reg.cells.x*3.7/1000.0,U_WH);stepTag="Wh";return;}
  if(st==0){st=1;stepTag="mAh?";tapeAdd("enter mAh");return;}
  double x=curValue();
  if(st==1){wMah=x;reg.capacity.x=x;reg.capacity.set=true;stepTag="S?";st=2;setEntry(x);}
  else if(st==2){wCells=x;reg.cells.x=x;reg.cells.set=true;setReg(reg.energy,wMah*wCells*3.7/1000.0,U_WH);stepTag="Wh";st=0;}
}

static void batteryWh(){if(need(reg.capacity)&&need(reg.cells))setReg(reg.answer,reg.capacity.x*reg.cells.x*3.7/1000.0,U_WH);}
static void torquePower(){if(need(reg.torque)&&need(reg.rpm))setReg(reg.answer,2*PI_D*reg.rpm.x*reg.torque.x/60.0,U_W);}
static void speed(){if(!need(reg.rpm)||!need(reg.pitch))return;setReg(reg.answer,reg.rpm.x*reg.pitch.x*60/63360.0,U_NONE);}
static void force(){if(need(reg.weight))setReg(reg.answer,reg.weight.x*G_ACC,U_NONE);}
static void mass(){if(need(reg.weight))setReg(reg.answer,reg.weight.x/G_ACC,U_KG);}

static void slope(){
  if(stepSlope==0&&typed())stepSlope=1;
  if(stepSlope==0&&reg.rise.set&&reg.run.set){double s=reg.rise.x/reg.run.x;
    setReg(reg.angle,atan2(reg.rise.x,reg.run.x)*180/PI_D,U_DEG);setReg(reg.answer,s,U_NONE);tapeVal("pct",100*s,U_PCT);stepTag="slp";return;}
  if(stepSlope==0){stepSlope=1;stepTag="rise?";tapeAdd("enter rise");return;}
  double x=curValue();
  if(stepSlope==1){wRise=x;reg.rise.x=x;reg.rise.u=U_FT;reg.rise.set=true;stepTag="run?";tapeVal("rise",x,U_FT);stepSlope=2;setEntry(x);}
  else if(stepSlope==2){wRun=x;reg.run.x=x;reg.run.u=U_FT;reg.run.set=true;double s=wRun!=0?wRise/wRun:NAN;
    setReg(reg.angle,atan2(wRise,wRun)*180/PI_D,U_DEG);setReg(reg.answer,s,U_NONE);tapeVal("pct",100*s,U_PCT);stepTag="slp";stepSlope=0;}
}

static void angleSolve(){if(!need(reg.rise)||!need(reg.run))return;setReg(reg.angle,atan2(reg.rise.x,reg.run.x)*180/PI_D,U_DEG);}
static void riseFromAngle(){if(!need(reg.run)||!need(reg.angle))return;setReg(reg.rise,reg.run.x*tan(reg.angle.x*PI_D/180),U_FT);}
static void roof(){
  static uint8_t st=0;if(st==0&&typed())st=1;
  if(st==0&&reg.rise.set&&reg.run.set){setReg(reg.answer,hypot(reg.rise.x,reg.run.x),U_FT);tapeVal("pitch",atan2(reg.rise.x,reg.run.x)*180/PI_D,U_DEG);return;}
  if(st==0){st=1;stepTag="rise?";tapeAdd("enter rise");return;}
  double x=curValue();
  if(st==1){wRise=x;reg.rise.x=x;reg.rise.u=U_FT;reg.rise.set=true;stepTag="run?";st=2;setEntry(x);}
  else if(st==2){wRun=x;reg.run.x=x;reg.run.u=U_FT;reg.run.set=true;
    setReg(reg.answer,hypot(wRise,wRun),U_FT);tapeVal("pitch",atan2(wRise,wRun)*180/PI_D,U_DEG);stepTag="raf";st=0;}
}

static void stairs(){if(!need(reg.rise)||!need(reg.run))return;setReg(reg.answer,hypot(reg.rise.x,reg.run.x),U_FT);tapeVal("risers",reg.dist.set?ceil(reg.dist.x/reg.rise.x):ceil(reg.rise.x/7.5),U_NONE);}
static void stairCount(){if(!need(reg.rise))return;setReg(reg.answer,floor(reg.rise.x/7.5),U_NONE);}
static void board(){
  static uint8_t st=0;if(st==0&&typed())st=1;static double t=0,w=0,l=0;
  if(st==0&&reg.rise.set&&reg.run.set&&reg.dist.set){setReg(reg.answer,reg.rise.x*reg.run.x*reg.dist.x/12.0,U_FT);stepTag="bf";return;}
  if(st==0){st=1;stepTag="T in?";tapeAdd("enter thickness in");return;}
  double x=curValue();
  if(st==1){t=x;stepTag="W in?";tapeVal("T",x,U_IN);st=2;setEntry(x);}
  else if(st==2){w=x;stepTag="L ft?";tapeVal("W",x,U_IN);st=3;setEntry(x);}
  else if(st==3){l=x;setReg(reg.answer,t*w*l/12.0,U_FT);tapeVal("+10%",t*w*l/12.0*1.1,U_FT);stepTag="bf";st=0;}
}

static void concrete(){if(!need(reg.volume))return;setReg(reg.answer,reg.volume.x/27.0*1.10,U_YD3);}
// TEST: perim=48 h=8 doors=1 wins=2 coats=2 cov=350 → net=333 gal≈1.90
// Sequential wizard - each press consumes current entry (no RISE/RUN hijack).
// Steps: 1 perim 2 height 3 doors 4 wins 5 coats 6 coverage → gallons
// PAINT: first press queues; stamps (rise=perim,run=ht,dist=doors,angle=wins) still work
static void paint(){
  if(stepPaint==0&&typed())stepPaint=1;
  if(stepPaint==0&&reg.rise.set&&reg.run.set){
    double doors=reg.dist.set?reg.dist.x:0,wins=reg.angle.set?reg.angle.x:0;
    double net=reg.rise.x*reg.run.x-(doors*21+wins*15);if(net<0)net=0;
    double coats=reg.total.set?reg.total.x:1,cov=reg.price.set?reg.price.x:350;
    reg.area.x=net;reg.area.u=U_FT;reg.area.set=true;
    setReg(reg.answer,net*coats/cov,U_GAL);tapeVal("NET",net,U_FT);tapeVal("BUY",ceil(net*coats/cov),U_GAL);
    stepTag="gal";return;
  }
  if(stepPaint==1&&typed()){pPerim=pHt=pDoors=pWins=0;pCoats=1;pCov=350;}
  if(stepPaint==0){pPerim=pHt=pDoors=pWins=0;pCoats=1;pCov=350;stepPaint=1;stepTag="perim?";tapeAdd("enter perim");return;}
  double x=curValue();
  if(stepPaint>=3&&!typed()){stepPaint=6;x=0;} // bare press after height = finish with defaults
  if(stepPaint==1){pPerim=x;stepTag="ht?";tapeVal("perim",x,U_FT);stepPaint=2;setEntry(x);}
  else if(stepPaint==2){pHt=x;stepTag="doors?";tapeVal("ht",x,U_FT);stepPaint=3;setEntry(x);}
  else if(stepPaint==3){pDoors=typed()?x:0;stepTag="wins?";tapeVal("doors",x,U_NONE);stepPaint=4;setEntry(x);}
  else if(stepPaint==4){pWins=typed()?x:0;stepTag="coats?";tapeVal("wins",x,U_NONE);stepPaint=5;setEntry(x);}
  else if(stepPaint==5){pCoats=typed()&&x>0?x:1;stepTag="cov?";tapeVal("coats",pCoats,U_NONE);stepPaint=6;setEntry(x);}
  else if(stepPaint==6){
    pCov=typed()&&x>0?x:350;tapeVal("cov",pCov,U_NONE);
    double net=pPerim*pHt-(pDoors*21+pWins*15);if(net<0)net=0;
    double g=net*pCoats/pCov;
    reg.area.x=net;reg.area.u=U_FT;reg.area.set=true;
    setReg(reg.answer,g,U_GAL);tapeVal("NET",net,U_FT);tapeVal("BUY",ceil(g),U_GAL);
    stepTag="gal";stepPaint=0;
  }
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

static void bake(){
  if(stepBake==0&&typed())stepBake=1;
  if(stepBake==0&&reg.flour.set&&reg.hydration.set){setReg(reg.answer,reg.flour.x*reg.hydration.x/100,U_G);stepTag="H2O";return;}
  if(stepBake==0){stepBake=1;stepTag="flour?";tapeAdd("enter flour g");return;}
  double x=curValue();
  if(stepBake==1){wFlour=x;reg.flour.x=x;reg.flour.u=U_G;reg.flour.set=true;stepTag="hydr%?";tapeVal("flour",x,U_G);stepBake=2;setEntry(x);}
  else if(stepBake==2){wHydr=x;reg.hydration.x=x;reg.hydration.u=U_PCT;reg.hydration.set=true;
    setReg(reg.answer,wFlour*wHydr/100,U_G);tapeVal("dough",wFlour+wFlour*wHydr/100,U_G);stepTag="H2O";stepBake=0;}
}

static void recipe(){if(!need(reg.total)||!need(reg.servings))return;setReg(reg.answer,reg.total.x/reg.servings.x,U_G);}
static void hydWater(){if(need(reg.flour)&&need(reg.hydration))setReg(reg.answer,reg.flour.x*reg.hydration.x/100,U_G);}
static void recipePct(){if(need(reg.total)&&need(reg.flour))setReg(reg.answer,reg.total.x/reg.flour.x*100,U_PCT);}
static void density(){if(need(reg.total)&&need(reg.servings))setReg(reg.answer,reg.total.x/reg.servings.x,U_G);}

static void loan(){
  static uint8_t st=0;if(st==0&&typed())st=1;
  if(st==0&&reg.principal.set&&reg.rate.set&&reg.term.set){
    double P=reg.principal.x,rm=reg.rate.x/100/12,n=reg.term.x;
    double pay=fabs(rm)<1e-15?P/n:P*rm*pow(1+rm,n)/(pow(1+rm,n)-1);
    reg.payment.x=pay;reg.payment.u=U_DOLLAR;reg.payment.set=true;reg.interest.x=pay*n-P;reg.interest.u=U_DOLLAR;reg.interest.set=true;
    setEntry(pay);tapeVal("PMT",pay,U_DOLLAR);tapeVal("INT",reg.interest.x,U_DOLLAR);stepTag="PMT";return;
  }
  if(st==0){st=1;stepTag="PV?";tapeAdd("enter principal");return;}
  double x=curValue();
  if(st==1){wP=x;reg.principal.x=x;reg.principal.u=U_DOLLAR;reg.principal.set=true;stepTag="APR%?";tapeVal("P",x,U_DOLLAR);st=2;setEntry(x);}
  else if(st==2){wRate=x;reg.rate.x=x;reg.rate.u=U_PCT;reg.rate.set=true;stepTag="mo?";tapeVal("APR",x,U_PCT);st=3;setEntry(x);}
  else if(st==3){wN=x;reg.term.x=x;reg.term.u=U_NONE;reg.term.set=true;
    double rm=wRate/100/12,pay=fabs(rm)<1e-15?wP/wN:wP*rm*pow(1+rm,wN)/(pow(1+rm,wN)-1);
    reg.payment.x=pay;reg.payment.u=U_DOLLAR;reg.payment.set=true;reg.interest.x=pay*wN-wP;reg.interest.u=U_DOLLAR;reg.interest.set=true;
    setEntry(pay);tapeVal("PMT",pay,U_DOLLAR);tapeVal("INT",reg.interest.x,U_DOLLAR);stepTag="PMT";st=0;}
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
static void statMinMax(){if(stN==0){beep();return;}char b[48];snprintf(b,48,"min%.4g max%.4g",stMin,stMax);tapeAdd(b);setEntry(stMin);tapeVal("sum",stSum,U_NONE);tapeVal("mean",stSum/stN,U_NONE);}
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
  case A_SIPREFIX:siPrefix();break;
  case A_MV:setReg(reg.v,x*1e-3,U_V);break;case A_MA:setReg(reg.a,x*1e-3,U_A);break;case A_KOHM:setReg(reg.r,x*1e3,U_OHM);break;
  case A_UF:setReg(reg.c,x*1e-6,U_F);break;case A_MH:setReg(reg.l,x*1e-3,U_H);break;
  case A_ALCU:useAl=!useAl;tapeAdd(useAl?"wire: ALUMINUM":"wire: COPPER");break;
  case A_TEMPCONV:if(reg.temp.set&&!typed()){if(reg.temp.u==U_FAH)setReg(reg.temp,(reg.temp.x-32)*5/9,U_C);else setReg(reg.temp,reg.temp.x*9/5+32,U_FAH);}
    else{tapeVal("C",x,U_C);setReg(reg.temp,x*9/5+32,U_FAH);}break;case A_ENG:engMode=!engMode;tapeAdd(engMode?"ENG ON":"ENG OFF");setEntry(curValue());break;
  default:break;
  }
  if(shift&&a!=A_NONE&&a!=A_ADD&&a!=A_SUB&&a!=A_MUL&&a!=A_DIV&&a!=A_EQ)shift=false;
  mark();
}

// === UI === skeuomorphic renderer (2005 body, 2010 pixel-perfect gloss)
// Everything is drawn into a PSRAM canvas, then pushed in one blit (no flicker).
// Geometry (touch zones) is unchanged: LCD 0..54, F-keys 56..122, numpad 128..216, bar 218..240.
static M5Canvas canvas(&M5.Display);
static bool useCanvas=false;
static int pressId=-1;static unsigned long pressUntil=0;   // key-down animation: 0-11 fkeys, 100+ numpad, 200+ bar

static constexpr uint16_t RGB(uint8_t r,uint8_t g,uint8_t b){return (uint16_t)(((r&0xF8)<<8)|((g&0xFC)<<3)|(b>>3));}
static uint16_t mix(uint16_t a,uint16_t b,int t){ // t 0..256
  int ar=a>>11,ag=(a>>5)&63,ab=a&31,br=b>>11,bg=(b>>5)&63,bb=b&31;
  return (uint16_t)(((ar+(br-ar)*t/256)<<11)|((ag+(bg-ag)*t/256)<<5)|(ab+(bb-ab)*t/256));
}
static constexpr uint16_t WHITE=0xFFFF,BLACK=0x0000;
// palette (from MantisCalc logo + mockup)
static constexpr uint16_t LCD_T=RGB(196,212,186),LCD_B=RGB(158,178,150),LCD_INK=RGB(24,36,32),LCD_GHOST=RGB(166,186,158),LCD_DIM=RGB(96,114,100);
struct KeyPal{uint16_t top,bot,edge,ink;};
static constexpr KeyPal P_FN ={RGB(66,152,152),RGB(14,72,76), RGB(4,28,30), WHITE};
static constexpr KeyPal P_DIG={RGB(92,104,112),RGB(34,42,48), RGB(10,14,16),WHITE};
static constexpr KeyPal P_OP ={RGB(156,74,166),RGB(68,12,78), RGB(30,2,36), WHITE};
static constexpr KeyPal P_EQ ={RGB(218,242,96),RGB(116,156,8), RGB(44,64,0), RGB(20,34,0)};
static constexpr uint16_t LIME=RGB(196,232,40),LIME_DIM=RGB(150,178,60);

template<class G> static void gradRR(G&g,int x,int y,int w,int h,int r,uint16_t top,uint16_t bot){
  if(h<=0||w<=0)return;
  for(int i=0;i<h;i++){
    int in=0,dy=-1;
    if(i<r)dy=r-i;else if(i>=h-r)dy=i-(h-r)+1;
    if(dy>0){float d=dy-0.5f;in=r-(int)(sqrtf((float)(r*r)-d*d)+0.5f);}
    g.drawFastHLine(x+in,y+i,w-2*in,h>1?mix(top,bot,i*256/(h-1)):top);
  }
}
template<class G> static void shadowText(G&g,int x,int y,const char*s,uint16_t ink,uint16_t shade){
  g.setTextColor(shade);g.setCursor(x+1,y+1);g.print(s);g.setTextColor(ink);g.setCursor(x,y);g.print(s);
}
template<class G> static void centerText(G&g,int x,int w,int y,const char*s,uint16_t ink,uint16_t shade){
  shadowText(g,x+(w-(int)g.textWidth(s))/2,y,s,ink,shade);
}
// Glossy domed key. Pressed: sinks 1px, shadow vanishes, gloss dims.
template<class G> static void glossKey(G&g,int x,int y,int w,int h,const KeyPal&p,bool down){
  const int r=5;
  if(!down)gradRR(g,x+1,y+2,w,h,r,RGB(8,10,12),RGB(4,4,6));          // drop shadow
  if(down)y+=1;
  gradRR(g,x,y,w,h,r,p.edge,p.edge);                                   // rim
  uint16_t t=down?mix(p.top,BLACK,70):p.top,b=down?mix(p.bot,BLACK,40):p.bot;
  gradRR(g,x+1,y+1,w-2,h-2,r-1,t,b);                                   // body
  if(!down){
    int gh=(h-4)/2;
    gradRR(g,x+2,y+2,w-4,gh,r-2,mix(t,WHITE,52),mix(t,WHITE,10));     // upper gloss lens
    g.drawFastHLine(x+r,y+1,w-2*r,mix(t,WHITE,170));                   // specular edge
  }
  g.drawFastHLine(x+r,y+h-2,w-2*r,mix(b,BLACK,90));                    // lower lip
}

// --- 7-segment LCD digits (11x20 cell, 14px pitch, crisp at native res)
static uint8_t segMask(char c){
  switch(c){case '0':return 63;case '1':return 6;case '2':return 91;case '3':return 79;case '4':return 102;
  case '5':return 109;case '6':return 125;case '7':return 7;case '8':return 127;case '9':return 111;
  case '-':return 64;case 'E':return 121;case 'r':return 80;default:return 0;}
}
template<class G> static void seg7(G&g,int x,int y,uint8_t m,uint16_t c){
  if(m&1)g.fillRect(x+2,y,7,2,c);       if(m&2)g.fillRect(x+9,y+2,2,7,c);   if(m&4)g.fillRect(x+9,y+11,2,7,c);
  if(m&8)g.fillRect(x+2,y+18,7,2,c);    if(m&16)g.fillRect(x,y+11,2,7,c);   if(m&32)g.fillRect(x,y+2,2,7,c);
  if(m&64)g.fillRect(x+2,y+9,7,2,c);
}
template<class G> static void lcdNumber(G&g,const char*src,int rightX,int y){
  char s[48];int n=0;               // normalise: e->E, drop '+', ERR->Err
  for(const char*p=src;*p&&n<46;p++){char c=*p;if(c=='+')continue;if(c=='e')c='E';if(c=='R')c='r';s[n++]=c;}s[n]=0;
  const int CELLS=16,P=14;
  int cells=0;for(int i=0;i<n;i++)if(s[i]!='.')cells++;
  int skip=cells>CELLS?cells-CELLS:0;          // overflow: show rightmost cells
  int x0=rightX-CELLS*P;
  for(int k=0;k<CELLS;k++){seg7(g,x0+k*P,y,127,LCD_GHOST);g.fillRect(x0+k*P+12,y+18,2,2,LCD_GHOST);}
  int k=CELLS-(cells-skip),seen=0;
  for(int i=0;i<n;i++){
    if(s[i]=='.'){if(seen>skip&&k>0)g.fillRect(x0+(k-1)*P+12,y+18,2,2,LCD_INK);continue;}
    if(seen++<skip)continue;
    seg7(g,x0+k*P,y,segMask(s[i]),LCD_INK);k++;
  }
}
template<class G> static void annun(G&g,int x,int y,const char*s,bool on,bool inv=false){
  if(inv&&on){int w=g.textWidth(s)+4;g.fillRoundRect(x-2,y-1,w,10,2,LCD_INK);g.setTextColor(LCD_T);}
  else g.setTextColor(on?LCD_INK:LCD_GHOST);
  g.setCursor(x,y);g.print(s);
}

template<class G> static void bezel(G&g){
  for(int y=0;y<H;y++){                                  // brushed gunmetal, deterministic grain
    uint32_t h=(uint32_t)y*2654435761u;int grain=(int)((h>>27)&7)-3;
    int v=58-y/10+grain;if(v<18)v=18;
    g.drawFastHLine(0,y,W,RGB(v,v+4,v+8));
  }
}
template<class G> static void lcdPanel(G&g,int x,int y,int w,int h){
  g.fillRoundRect(x-1,y-1,w+2,h+2,6,RGB(8,10,12));      // recess
  g.drawFastHLine(x+4,y+h+1,w-8,RGB(120,128,136));       // lit lower lip of the bezel cut
  gradRR(g,x+1,y+1,w-2,h-2,4,LCD_T,LCD_B);               // glass
  g.drawFastHLine(x+3,y+1,w-6,RGB(112,126,108));         // inner top shadow
  g.drawFastVLine(x+1,y+3,h-6,RGB(132,148,126));
}
template<class G> static void drawFnKey(G&g,int i,const Key&k){
  int x=(i%6)*53+1,y=FUNC_Y+(i/6)*34;bool down=pressId==i;
  glossKey(g,x,y,51,31,P_FN,down);if(down)y++;
  g.setTextSize(1);
  const char*big=shift&&k.s[0]?k.s:k.p,*small=shift?k.p:k.s;
  if(small&&small[0])centerText(g,x,51,y+4,small,shift?RGB(150,190,190):LIME,RGB(0,40,42));
  centerText(g,x,51,y+17,big,shift?LIME:WHITE,RGB(0,30,32));
}
template<class G> static void drawNumKey(G&g,int i,const char*p,const char*s){
  int row=i/4,col=i%4,x=2+col*79,y=NUM_Y+row*22;bool down=pressId==100+i;
  const KeyPal&pal=i==14?P_EQ:(col==3?P_OP:P_DIG);
  glossKey(g,x,y,76,20,pal,down);if(down)y++;
  g.setTextSize(1);
  uint16_t shade=i==14?RGB(230,250,160):BLACK;
  if(shift){centerText(g,x,76,y+6,s,i==14?RGB(60,0,70):LIME,shade);g.setTextColor(mix(pal.ink,pal.bot,140));g.setCursor(x+5,y+6);g.print(p);}
  else{centerText(g,x,76,y+6,p,pal.ink,shade);g.setTextColor(i==14?RGB(70,10,80):LIME_DIM);g.setCursor(x+5,y+6);g.print(s);}
}
template<class G> static void barPill(G&g,int id,int x,int w,const char*s,bool lit){
  bool down=pressId==200+id;int y=BAR_Y+3+(down?1:0);
  g.fillRoundRect(x,y,w,17,8,RGB(10,12,14));                                    // engraved slot
  gradRR(g,x+1,y+1,w-2,15,7,lit?RGB(130,40,140):RGB(56,64,72),lit?RGB(64,8,72):RGB(26,30,34));
  if(!down)g.drawFastHLine(x+8,y+2,w-16,lit?RGB(190,110,200):RGB(100,110,118));
  g.drawFastHLine(x+8,y+17,w-16,RGB(96,104,112));
  g.setTextSize(1);centerText(g,x,w,y+5,s,lit?LIME:RGB(220,226,230),BLACK);
}
template<class G> static void drawBar(G&g,bool help){
  g.drawFastHLine(0,BAR_Y,W,RGB(14,16,18));g.drawFastHLine(0,BAR_Y+1,W,RGB(84,92,100));
  char a[16],c[16];
  snprintf(a,16,"< %s",pageNames[(page+5)%6]);snprintf(c,16,"%s >",pageNames[(page+1)%6]);
  barPill(g,0,6,92,a,false);
  barPill(g,1,118,84,help?"EXIT":"2ND",!help&&shift);
  barPill(g,2,222,92,c,false);
}
template<class G> static void drawAll(G&g){
  bezel(g);
  if(helpMode){
    lcdPanel(g,4,3,312,210);
    g.fillRoundRect(8,7,304,13,3,LCD_INK);g.setTextSize(1);g.setTextColor(LCD_T);
    g.setCursor(12,10);g.printf("GUIDE  %s  %d/6",pageNames[page],page+1);
    g.setTextColor(LCD_INK);
    for(int i=0,y=26;i<14;i++){const char*l=helpText[page][i];if(!l||!l[0])continue;g.setCursor(10,y);g.print(l);y+=13;}
    drawBar(g,true);return;
  }
  // --- LCD
  lcdPanel(g,4,2,312,51);
  g.setTextSize(1);
  annun(g,9,6,pageNames[page],true,true);
  int ax=13+g.textWidth(pageNames[page]);
  annun(g,ax,6,"2ND",shift,true);ax+=24;
  annun(g,ax,6,"M",memory!=0);ax+=10;
  annun(g,ax,6,"ENG",engMode);ax+=22;
  if(page==1){annun(g,ax,6,useAl?"Al":"Cu",true);ax+=16;}
  if(stepTag[0]){annun(g,ax,6,stepTag,true,true);}
  if(page==1){                                            // ELEC register annunciators
    const char*labs[]={"V","A","R","P","C","L","f","d"};
    Value*vs[]={&reg.v,&reg.a,&reg.r,&reg.p,&reg.c,&reg.l,&reg.hz,&reg.dist};
    for(int i=0;i<8;i++)annun(g,262+i*7,6,labs[i],vs[i]->set);
  }
  if(op){char a[20],b[26];fmt(cur,a,sizeof a);snprintf(b,sizeof b,"%s %c",a,op=='%'?'m':op);annun(g,9,22,b,true);}
  char n[40];
  if(entering)snprintf(n,sizeof n,"%s",entry);else fmt(curValue(),n,sizeof n);
  lcdNumber(g,n,310,15);
  g.drawFastHLine(8,38,304,LCD_GHOST);                    // tape line inside the glass
  if(tapeCount>0){g.setTextColor(LCD_INK);g.setCursor(9,42);g.print(tape[0]);}
  // --- keys
  for(int i=0;i<12;i++)drawFnKey(g,i,pages[page][i]);
  static const char*nums[16]={"7","8","9","/","4","5","6","*","1","2","3","-","0",".","=","+"};
  static const char*secs[16]={"pi","e","%","^","BS","+/-","ABS","mod","1/x","ANS","EE","d%","CLR","MR","MS","MC"};
  for(int i=0;i<16;i++)drawNumKey(g,i,nums[i],secs[i]);
  drawBar(g,false);
}
static void draw(){
  if(!dirty)return;dirty=false;
  if(useCanvas){drawAll(canvas);canvas.pushSprite(0,0);}else drawAll(M5.Display);
}
static void pressFx(int id){pressId=id;pressUntil=millis()+110;mark();}
static void touch(){
  auto t=M5.Touch.getDetail();if(!t.wasPressed())return;
  lastTouch=millis();
  int x=t.x,y=t.y;if(y>=H||x<0||x>=W)return;vib();
  // 2ND + tap header opens on-device guide for this page
  if(y<TOP){
    if(shift||helpMode){helpMode=!helpMode;helpScroll=0;shift=false;vib();mark();}
    return;
  }
  if(helpMode){
    // A/C zones or bar: cycle pages; center = exit
    if(y>=BAR_Y){
      if(x<106){page=(page+5)%6;helpScroll=0;}
      else if(x>212){page=(page+1)%6;helpScroll=0;}
      else{helpMode=false;}
      mark();return;
    }
    // tap left/right half of screen also cycles
    if(x<W/2){page=(page+5)%6;helpScroll=0;}
    else{page=(page+1)%6;helpScroll=0;}
    mark();return;
  }
  if(y>=BAR_Y){
    if(x<106){pressFx(200);page=(page+5)%6;clearEntry();shift=false;helpMode=false;}
    else if(x>212){pressFx(202);page=(page+1)%6;clearEntry();shift=false;helpMode=false;}
    else{pressFx(201);shift=!shift;}
    mark();return;
  }
  if(y>=FUNC_Y&&y<FUNC_Y+68){
    int col=x/53;if(col>5)col=5;int row=(y-FUNC_Y)/34,i=row*6+col;
    if(i>=0&&i<12){pressFx(i);Key&k=pages[page][i];doAct(shift&&k.b!=A_NONE?k.b:k.a);}
    return;
  }
  if(y>=NUM_Y){
    int row=(y-NUM_Y)/22,col=x/79;
    if(row<4&&col<4){
      int i=row*4+col;pressFx(100+i);
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
  canvas.setColorDepth(16);canvas.setPsram(true);useCanvas=canvas.createSprite(W,H)!=nullptr;
  if(useCanvas)canvas.setTextWrap(false);
  initAwg();splash();dirty=true;draw();
}
void loop(){
  M5.update();
  if(M5.BtnA.wasPressed()){
    if(helpMode){page=(page+5)%6;helpScroll=0;}
    else{page=(page+5)%6;clearEntry();shift=false;}
    pressFx(200);vib();mark();
  }
  if(M5.BtnC.wasPressed()){
    if(helpMode){page=(page+1)%6;helpScroll=0;}
    else{page=(page+1)%6;clearEntry();shift=false;}
    pressFx(202);vib();mark();
  }
  if(M5.BtnB.wasPressed()){
    if(helpMode)helpMode=false;
    else shift=!shift;
    pressFx(201);vib();mark();
  }
  touch();
  if(pressId>=0&&(long)(millis()-pressUntil)>=0){pressId=-1;mark();}
  static unsigned long last=0;
  if(dirty||millis()-last>200){draw();last=millis();}
  delay(4);
}
