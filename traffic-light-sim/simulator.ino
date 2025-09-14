/**
 * @file simulator.ino
 * @brief Traffic light simulation that mimics the
 * actions of a set of traffic and pedestrian lights
 * with PIR sensor and a button for requests.
 */

/* --- DECLARATIONS --- */

// Constant variables:
// These are mostly pins, bps, delays,
// light timings etc.

const int BPS_FOR_SERIAL = 9600;
const int DELAY_BETWEEN_SWITCH = 1000;

const int TRAFFIC_RED_PIN = 13;
const int TRAFFIC_YELLOW_PIN = 12;
const int TRAFFIC_GREEN_PIN = 11;

const int PED_RED_PIN = 3;
const int PED_GREEN_PIN = 2;

const int PED_BUTTON_PIN = 5;
const int PIR_SENSOR_PIN = 6;

const unsigned long TRAFFIC_GREEN_TIME = 10000;
const unsigned long TRAFFIC_YELLOW_TIME = 3000;
const unsigned long TRAFFIC_RED_TIME = 8000;
const unsigned long PED_GREEN_TIME = 8000;

// State variables
// These variables are changed throughout the code.
// And to keep track of what is going on under the code i.e.
// used in the debugging function: print_status()

int button_state = LOW;
int last_button_state = LOW;
int pir_state = LOW;

unsigned long state_start_time = 0;
unsigned long last_pir_trigger = 0;

bool pedestrian_requested = false;
bool pir_detected = false;

// Enumerate to keep track of
// the current state of the traffic.

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
 * @param I do not want to write what every one of them
 * does. Simply given the state its corresponding pin is
 * changed.
 */
static void set_lights(int traf_red, int traf_yellow, int traf_green, int ped_red, int ped_green) {
    
    digitalWrite(TRAFFIC_RED_PIN, traf_red);
    digitalWrite(TRAFFIC_YELLOW_PIN, traf_yellow);
    digitalWrite(TRAFFIC_GREEN_PIN, traf_green);
    digitalWrite(PED_RED_PIN, ped_red);
    digitalWrite(PED_GREEN_PIN, ped_green);
}

/**
 * @brief Checks pedestrian button input and PIR sensor
 */
static void check_inputs() {
	
	// CHECKING BUTTON STATE:
	
    int new_button_state = digitalRead(PED_BUTTON_PIN);
    
    if (new_button_state == LOW && last_button_state == HIGH) {
        pedestrian_requested = true;
        Serial.println("Pedestrian crossing requested via button.");
    }
	
    last_button_state = new_button_state;
    
    // CHECKING PIR SENSOR STATE:
	
    int new_pir_state = digitalRead(PIR_SENSOR_PIN);
    
    if (new_pir_state == HIGH && pir_state == LOW) {
		
        pir_detected = true;
        last_pir_trigger = millis();
        Serial.println("Motion detected by PIR sensor.");
        
        if (current_state == TRAFFIC_GREEN) {
            pedestrian_requested = true;
            Serial.println("Pedestrian crossing requested via PIR sensor");
		}
    }
	
    pir_state = new_pir_state;
}

/**
 * @brief Handle state transitions based on timing and inputs
 */
static void handle_state_machine() {
	
    unsigned long current_time = millis();
    unsigned long elapsed_time = current_time - state_start_time;
    
    switch(current_state) {
		
        case TRAFFIC_GREEN:
		
            set_lights(LOW, LOW, HIGH, HIGH, LOW);
            
			// Even though a pedestrian requested to pass
			// Allow green to stay for a minimum of 5 seconds.
			// We don't want any crashing happening after all :-) lol
			
            if ((pedestrian_requested && elapsed_time >= 5000) || elapsed_time >= TRAFFIC_GREEN_TIME) {
                
				current_state = TRAFFIC_YELLOW;
                state_start_time = current_time;
                Serial.println("State: TRAFFIC_YELLOW");
			}
			
            break;
            
        case TRAFFIC_YELLOW:
		
            set_lights(LOW, HIGH, LOW, HIGH, LOW);
            
            if (elapsed_time >= TRAFFIC_YELLOW_TIME) {
				
                current_state = TRAFFIC_RED;
                state_start_time = current_time;
                Serial.println("State: TRAFFIC_RED");
				
            }
            break;
            
        case TRAFFIC_RED:
		
			// I have initialized both pedestrian and traffic
			// to red. Because unless a pedestrian requests
			// or PIR sensor detects someone there is no need for
			// redundant green light for pedestrians.
            set_lights(HIGH, LOW, LOW, HIGH, LOW);
            
			// PEDESTRIAN REQUEST
            if (pedestrian_requested && elapsed_time >= 1000) {
                current_state = PED_GREEN;
                state_start_time = current_time;
                pedestrian_requested = false;  // Clear the request
                Serial.println("State: PED_GREEN");
            }
			
            // OTHERWISE CONTINUE LOOP
            else if (!pedestrian_requested && elapsed_time >= TRAFFIC_RED_TIME) {
                current_state = TRAFFIC_GREEN;
                state_start_time = current_time;
                Serial.println("State: TRAFFIC_GREEN");
            }
            break;
            
        case PED_GREEN:
		
            set_lights(HIGH, LOW, LOW, LOW, HIGH);
            
            if (elapsed_time >= PED_GREEN_TIME) {
                current_state = TRAFFIC_GREEN;
                state_start_time = current_time;
                Serial.println("State: TRAFFIC_GREEN");
            }
			
            break;
            
        default:
		
            // Theoretically should never reach here.
			// But to be safe, I wrote these.
			
            current_state = TRAFFIC_RED;
            state_start_time = current_time;
            Serial.println("ERROR: Unknown state, resetting to TRAFFIC_RED");
			
            break;
    }
}

/**
 * @brief Print system status for debugging
 */
static void print_status() {
	
    static unsigned long last_status_print = 0;
    unsigned long current_time = millis();
    
    if (current_time - last_status_print >= 2000) {
        
		Serial.print("Current State: ");
        
		switch(current_state) {
			
            case TRAFFIC_GREEN:
				Serial.print("TRAFFIC_GREEN");
				break;
				
            case TRAFFIC_YELLOW:
				Serial.print("TRAFFIC_YELLOW");
				break;
				
            case TRAFFIC_RED:
				Serial.print("TRAFFIC_RED");
				break;
            
			case PED_GREEN:
				Serial.print("PED_GREEN");
				break;
        }
        
        Serial.print(" | Button State: ");
        Serial.print(last_button_state ? "HIGH" : "LOW");
		
        Serial.print(" | PIR Active: ");
        Serial.print(pir_state ? "YES" : "NO");
		
        Serial.print(" | Ped Requested: ");
        Serial.print(pedestrian_requested ? "YES" : "NO");
		
        unsigned long elapsed = millis() - state_start_time;
		
        Serial.print(" | Elapsed: ");
        Serial.print(elapsed);
        Serial.println("ms");
        
        last_status_print = current_time;
    }
}

/* --- Main Setup --- */
void setup() {
	
	// LED PINS
    pinMode(TRAFFIC_RED_PIN, OUTPUT);
    pinMode(TRAFFIC_YELLOW_PIN, OUTPUT);
    pinMode(TRAFFIC_GREEN_PIN, OUTPUT);
    pinMode(PED_RED_PIN, OUTPUT);
    pinMode(PED_GREEN_PIN, OUTPUT);
    
	// BUTTONS & SENSORS
    pinMode(PED_BUTTON_PIN, INPUT);
    pinMode(PIR_SENSOR_PIN, INPUT);
    
    // For beginning all LEDs are OFF.
    set_lights(LOW, LOW, LOW, LOW, LOW);
    
    // Serial communication for debug
    Serial.begin(BPS_FOR_SERIAL);
    
    // Start timing
    state_start_time = millis();
    
    // Start with traffic green
    current_state = TRAFFIC_GREEN;
    
    Serial.println("Traffic Light Simulation :-)!");
    Serial.println("Press pedestrian button or trigger PIR sensor to request crossing");
	Serial.println("Otherwise you are doomed to stay here. :(");
}

/* --- Main Loop --- */
void loop() {
    check_inputs();
    handle_state_machine();
    print_status();
}
