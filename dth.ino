#define MAXTIMINGS 85

#define cli noInterrupts
#define sei interrupts

#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21

#define NAN 999999

class DHT {
    private:
        uint8_t data[6];
        uint8_t _pin, _type, _count;
        bool read(void);
        unsigned long _lastreadtime;
        bool firstreading;

    public:
        DHT(uint8_t pin, uint8_t type, uint8_t count=6);
        void begin(void);
        float readTemperature(bool S=false);
        float convertCtoF(float);
        float readHumidity(void);

};


DHT::DHT(uint8_t pin, uint8_t type, uint8_t count) {
    _pin = pin;
    _type = type;
    _count = count;
    firstreading = true;
}


void DHT::begin(void) {
    // set up the pins!
    pinMode(_pin, INPUT);
    digitalWrite(_pin, HIGH);
    _lastreadtime = 0;
}


//boolean S == Scale.  True == Farenheit; False == Celcius
float DHT::readTemperature(bool S) {
    float _f;

    if (read()) {
        switch (_type) {
            case DHT11:
                _f = data[2];

                if(S)
                    _f = convertCtoF(_f);

                return _f;


            case DHT22:
            case DHT21:
                _f = data[2] & 0x7F;
                _f *= 256;
                _f += data[3];
                _f /= 10;

                if (data[2] & 0x80)
                    _f *= -1;

                if(S)
                    _f = convertCtoF(_f);

                return _f;
        }
    }

    return NAN;
}


float DHT::convertCtoF(float c) {
    return c * 9 / 5 + 32;
}


float DHT::readHumidity(void) {
    float _f;
    if (read()) {
        switch (_type) {
            case DHT11:
                _f = data[0];
                return _f;


            case DHT22:
            case DHT21:
                _f = data[0];
                _f *= 256;
                _f += data[1];
                _f /= 10;
                return _f;
        }
    }

    return NAN;
}


bool DHT::read(void) {
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    unsigned long currenttime;

    // pull the pin high and wait 250 milliseconds
    digitalWrite(_pin, HIGH);
    delay(250);

    currenttime = millis();
    if (currenttime < _lastreadtime) {
        // ie there was a rollover
        _lastreadtime = 0;
    }

    if (!firstreading && ((currenttime - _lastreadtime) < 2000)) {
        //delay(2000 - (currenttime - _lastreadtime));
        return true; // return last correct measurement
    }

    firstreading = false;
    Serial.print("Currtime: "); Serial.print(currenttime);
    Serial.print(" Lasttime: "); Serial.print(_lastreadtime);
    _lastreadtime = millis();

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    // now pull it low for ~20 milliseconds
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    delay(20);
    cli();
    digitalWrite(_pin, HIGH);
    delayMicroseconds(40);
    pinMode(_pin, INPUT);

    // read in timings
    for ( i=0; i< MAXTIMINGS; i++) {
        counter = 0;

        while (digitalRead(_pin) == laststate) {
            counter++;
            delayMicroseconds(1);

            if (counter == 255)
                break;
        }

        laststate = digitalRead(_pin);

        if (counter == 255)
            break;

        // ignore first 3 transitions
        if ((i >= 4) && (i%2 == 0)) {
            // shove each bit into the storage bytes
            data[j/8] <<= 1;

            if (counter > _count)
                data[j/8] |= 1;

            j++;
        }
    }

    sei();


    // check we read 40 bits and that the checksum matches
    if ((j >= 40) &&  (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)))
        return true;


    return false;
}

#define DHTPIN D2    // Digital pin D2

// IMPORTANT !! Make sure you set this to your 
// sensor type.  Options: [DHT11, DHT22, DHT21, AM2301]
#define DHTTYPE DHT22  

DHT dht(DHTPIN, DHTTYPE);

float h;      // humidity
float t;      // temperature
char h1[10];  // humidity string
char t1[10];  // temperature string
int f = 0;    // failed?

void setup() {
    Spark.variable("humidity", &h1, STRING);
    Spark.variable("temperature", &t1, STRING);
    Spark.variable("status", &f, INT);
    dht.begin();
} 

void loop() {
    f = 0;
    h = dht.readHumidity();
    t = dht.readTemperature();

    if (t==NAN || h==NAN) {
        f = 1; // not a number, fail.
    }
    else {
        f = 0; // both numbers! not failed.
        sprintf(h1, "%.2f", h); // convert Float to String
        sprintf(t1, "%.2f", t);
    }
}
