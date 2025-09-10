/* --- DECLARATIONS --- */

const int BPS_FOR_SERIAL = 9600;
const int DELAY_BETWEEN_SWITCH = 1000;

const int TRAFFIC_RED_PIN = 13;
const int TRAFFIC_YELLOW_PIN = 12;
const int TRAFFIC_GREEN_PIN = 11;

const int PED_RED_PIN = 3;
const int PED_GREEN_PIN = 2;

const int ALL_LEDS[] = {TRAFFIC_RED_PIN, TRAFFIC_YELLOW_PIN, TRAFFIC_GREEN_PIN,
			 PED_RED_PIN, PED_GREEN_PIN};

const int PED_BUTTON_PIN = 5;
const int PIR_SENSOR_PIN = 6;

const unsigned long TRAFFIC_GREEN_TIME = 10000;
const unsigned long TRAFFIC_YELLOW_TIME = 3000;
const unsigned long TRAFFIC_RED_TIME = 8000;
const unsigned long PED_GREEN_TIME = 8000; 

int button_state = LOW;
int last_button_state = LOW;
int pir_state = LOW;

unsigned long state_start_time = 0;
unsigned long last_pir_trigger = 0;

bool pedestrian_requested = false;
bool pir_detected = false;

typedef enum {
	TRAFFIC_GREEN,
	TRAFFIC_YELLOW,
	TRAFFIC_RED,
	PED_GREEN
} TrafficState;

TrafficState current_state = TRAFFIC_GREEN;

/* -------------------------- */
/* --- Internal Functions --- */
/* -------------------------- */

/**
 * @brief Configure the LEDs by the given parameters
 * @param pin_state Takes an integer to change the corresponding
 * pin state.
 */
static void set_lights(int traf_red_pin, int traf_yellow_pin,
				   int traf_green_pin, int ped_red_pin, int ped_green_pin) {
	
	int CURRENT_STATES[] = {traf_red_pin, traf_yellow_pin, traf_green_pin, 
						ped_red_pin, ped_green_pin};

	for (size_t i = 0; i < sizeof(ALL_LEDS) / sizeof(int); i++) {
		digitalWrite(ALL_LEDS[i], CURRENT_STATES[i]);
	}
}

/**
 * @brief Check pedestrian button input and PIR sensor
 * Makes pedestrian baised decisions.
 */
static void check_input() {
	
	int new_button_state = digitalRead(PED_BUTTON_PIN);
	
	if (new_button_state == HIGH && last_button_state == LOW) {
		
		pedestrian_requested = true;
		Serial.println("Pedestrian pressed button!");
	}
	
	last_button_state = new_button_state;
	
	int new_pir_state = digitalRead(PIR_SENSOR_PIN);
	
	if (new_pir_state == HIGH && pir_state == LOW) {
		
		pir_detected = true;
		last_pir_trigger = millis();
		Serial.println("Motion detected!");
		
		if (current_state == TRAFFIC_GREEN) {
			pedestrian_requested = true;
			Serial.println("RICH MUST WAIT BEFORE GOING. >:(");
		}
	}
	
	pir_state = new_pir_state;
}  

static void handle_state() {
	
	unsigned long current_time = millis();
	unsigned long elapsed_time = current_time - state_start_time;
	
	switch (current_state) {
		
		case TRAFFIC_GREEN:
		
			set_lights(LOW, LOW, HIGH, HIGH, LOW);
			
			if ((pedestrian_requested && elapsed_time >= 5000) || 
				elapsed_time >= TRAFFIC_GREEN_TIME) {
					
				current_state = TRAFFIC_YELLOW;
				state_start_time = current_time;
				Serial.println("Switching to traffic yellow!");
			}
		
			break;
		
		case TRAFFIC_YELLOW:
		
			set_lights(LOW, HIGH, LOW, HIGH, LOW);
			
			if (elapsed_time >= TRAFFIC_YELLOW_TIME) {
				
				current_state = TRAFFIC_RED;
				state_start_time = current_time;
				Serial.println("Switching to traffic red!");
			}
		
		case TRAFFIC_RED:
			
			
		
		case PED_GREEN:
		
		default:
			
			// Theorically program never reaches here.
			
			current_state = TRAFFIC_RED;
			state_start_time = current_time;
			break;
	}
}

/* --- Main Setup --- */

void setup() {
	
	pinMode(TRAFFIC_RED_PIN, OUTPUT);
	pinMode(TRAFFIC_YELLOW_PIN, OUTPUT);
	pinMode(TRAFFIC_GREEN_PIN, OUTPUT);
	
	pinMode(PED_RED_PIN, OUTPUT);
	pinMode(PED_GREEN_PIN, OUTPUT);
	
	pinMode(PED_BUTTON_PIN, INPUT);
	pinMode(PIR_SENSOR_PIN, INPUT);

	Serial.begin(BPS_FOR_SERIAL);
	
	state_start_time = millis();
	
	current_state = TRAFFIC_GREEN;
	
	Serial.println("### Traffic Lights ###");
	Serial.println("Press button to cross the street!! :-)");
}

/* --- Main Loop --- */

void loop() {
	check_input();
	handle_state();
	
	delay(50);
}