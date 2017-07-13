#include <Wire.h>
#include <rd02.h>

#define SERIAL_BAUDS 38400

/*------------------------*/
/*-        Timer2        -*/
/*------------------------*/

#define TIMER1_TOP (int)(0.001 * (16e6 / 64.0)) - 1 // 249
#define TIMER1_OVF_TIME_MS 1
#define SPEED_CHECK_CYCLE 4   // 4ms
#define SERIAL_PRINT_CYCLE 500   // 256ms

// timer variables
//volatile uint64_t milliseconds = 0;
volatile unsigned long int milliseconds = 0;
volatile bool speed_check_flag = false;
volatile bool serial_print_flag = false;

ISR(TIMER2_OVF_vect) {
  milliseconds++;
  if(milliseconds % SPEED_CHECK_CYCLE == 0) {
    speed_check_flag = true;
  }
  if(milliseconds % SERIAL_PRINT_CYCLE == 0) {
    serial_print_flag = true;
  }
}

void timer2_config() {
  cli();

  // Timer2 settings:
  //   - CTC mode
  //   - 64 prescaler
  TCCR2A = 0x0;
  TCCR2A |= (1<<(WGM21));
  TCCR2A &= ~(~(1<<(WGM20)) & ~(1<<(WGM22)));
  TCCR2A |= (1<<(COM2A1)) | (1<<(COM2B1));

  TCCR2B &= ~(~(1<<(CS20)) & ~(1<<(CS21)));
  TCCR2B |= (1<<(CS22));

  OCR2A = TIMER1_TOP;
  TCNT2 = 0;
  TIMSK2 |= (1<<(TOIE2));

  sei();
}

/*------------------------*/
/*-         RD02         -*/
/*------------------------*/
#define RD02_I2C_ADDRESS      byte((0xB0)>>1)

#define WHEEL_DIAMETRE_MM     1000

const int32_t CORRIDOR_LENGTH = 0x7FFFFFFF;

int32_t prev_enc1=0, prev_enc2=0;
int32_t enc1=0, enc2=0;

int speed_mms=0;

/*------------------------*/
/*-        SKETCH        -*/
/*------------------------*/

void setup()
{
  // start services
  Serial.begin(SERIAL_BAUDS);
  Wire.begin();
  timer2_config();

  // hardware delays

  // hardware configs

  Serial.print(F("configuring RD02 with address 0x"));
  Serial.print(RD02_I2C_ADDRESS<<1,HEX);
  Serial.println(F("..."));
  rd02::reset_encoders(RD02_I2C_ADDRESS);
  //rd02::disable_speed_regulation(RD02_I2C_ADDRESS);
  rd02::set_mode(RD02_I2C_ADDRESS, 1);
  rd02::set_speed1(RD02_I2C_ADDRESS, 0);
  rd02::set_speed2(RD02_I2C_ADDRESS, 0);
  Serial.println(F("done."));
}

int previous_revolutions = 0;
int revolutions = 0;
unsigned long ms_benchmarks[20] = {0};
int counter = 0;

// continuously reads packets, looking for ZB Receive or Modem Status
void loop()
{
  int32_t delta = 0;
  if(counter < 20) {
    rd02::set_speed2(RD02_I2C_ADDRESS,127);

    previous_revolutions = revolutions;

    if(speed_check_flag) {
      prev_enc2 = enc2;
      enc2 = rd02::read_enc2(RD02_I2C_ADDRESS);
      delta = enc2 - prev_enc2;
      speed_check_flag = false;
      revolutions = enc2 / 360;
    }
    if(previous_revolutions != revolutions) {
      ms_benchmarks[counter] = milliseconds;
      counter++;
    }
  }
  else {
    for(int i=0; i<20; i++) {
      //Serial.print("rev "); Serial.print(i);
      //Serial.print(" at ms "); Serial.println(ms_benchmarks[i]);
      Serial.println(ms_benchmarks[i]);
    }
    while(1);
  }
}

/***************************************/
// Results:
// Speed applied | ms per revolution
// - - - - - - - - - - - - - - - - -
// 7  ->  5000
// 15 ->  2500
// 31 -> 1197    | 1192   | 1200
//       2392      2389     2396
//       3584      3584     3588
//       4777      4776     4781
//       5969      5969     5972
//       7161      7161     7168
//       8357      8357     8361
//       9544      9553     9552
//       10737     10744    10744
//       11932     11937    11941
//       13124     13129    13132
//       14320     14328    14325
//       15509     15517    15520
//       16701     16712    16713
//       17892     17909    17905
//       19089     19104    19097
//       20281     20296    20297
//       21477     21493    21488
//       22665     22681    22681
//       23860     23873    23877
//
// 63 -> 596    | 596   | 728
//       1193     1192    1324
//       1788     1789    1916
//       2012     2384    2512
//       2017     2981    3108
//       2385     3577    3704
//       2981     4169    4272
//       3576     4765    4277
//       4172     5365    4300
//       4765     5961    4896
//       5360     6552    5493
//       5957     7148    6088
//       6553     7745    6685
//       7148     8341    7280
//       7744     8936    7877
//       8340     9533    8472
//       8936     10129   9065
//       9377     10725   9661
//       9380     11320   10256
//       9529     11917   10852
//
// 127 ->  309  | 540  | 528
//         613    848    836
//         757    1153   1141
//         760    1457   1444
//         917    1760   1748
//         1225   2068   2056
//         1529   2372   2360
//         1832   2676   2665
//         1972   2980   2968
//         1977   3289   3276
//         2137   3593   3580
//         2444   3897   3885
//         2748   4200   4189
//         3052   4257   4496
//         3356   4260   4801
//         3664   4509   5105
//         3969   4813   5409
//         4273   5117   5717
//         4576   5421   6020
//         4885   5729   6325
