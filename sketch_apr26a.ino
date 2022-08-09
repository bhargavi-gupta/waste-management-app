#include <Q2HX711.h>
// Including the GSM library
#include <GSM.h>

#define PINNUMBER ""

// initialize the library instance
GSM gsmAccess;
GSM_SMS sms;
long remoteNum = 0;

const byte hx711_data_pin = 3;
const byte hx711_clock_pin = 4;


float y1 = 20.0; // calibrated mass to be added( in grams)
long x1 = 0L;
long x0 = 0L;
float avg_size = 10.0; // amount of averages for each mass measurement

Q2HX711 hx711(hx711_data_pin, hx711_clock_pin); // prep hx711

void setup() {
  Serial.begin(9600); // prepare serial port
  delay(1000); // allow load cell and hx711 to settle
  // tare procedure
  for (int ii=0;ii<int(avg_size);ii++){
    delay(10);
    x0+=hx711.read();
  }
  x0/=long(avg_size);
  Serial.println("Add Calibrated Mass");
  // calibration procedure (mass should be added equal to y1)
  int ii = 1;
  while(true){
    if (hx711.read()<x0+10000){
    } else {
      ii++;
      delay(2000);
      for (int jj=0;jj<int(avg_size);jj++){
        x1+=hx711.read();
      }
      x1/=long(avg_size);
      break;
    }
  }
  Serial.println("Calibration Complete");
  
  //SETTING UP TEXT MESSAGE SYSTEM
  // initializing serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("SMS Messages Sender");

  // connection state
  boolean notConnected = true;

  // Start GSM shield
  while (notConnected) {
    if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
      notConnected = false;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("GSM initialized");
}

void loop() {
  // averaging reading
  long reading = 0;
  for (int jj=0;jj<int(avg_size);jj++){
    reading+=hx711.read();
  }
  reading/=long(avg_size);
  // calculating mass based on calibration and linear fit
  float ratio_1 = (float) (reading-x0);
  float ratio_2 = (float) (x1-x0);
  float ratio = ratio_1/ratio_2;
  float mass = y1*ratio;
  
  //for alerting is waste in the dustbin is greater than 1kg(1000 gms)
  if (mass > 1000.0){
    sms.beginSMS(remoteNum);// remoteNum = Mobile Number associated to Municipality Server
    sms.print("Waste collected.");
    sms.endSMS();
    Serial.println("\nNOTIFIED THE MUNICIPALITY SERVER.\n");
    delay(86400000UL);
}
}