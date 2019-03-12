#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT11


DHT_Unified dht(DHTPIN, DHTTYPE);

#pragma region
const byte SEGMENTS_COUNT = 8;
const byte NUMBER_OF_DIGITS = 4;

byte const CHAR_A = B11101110;
byte const CHAR_C = B10011100;
byte const CHAR_O = B11111100;
byte const CHAR_E = B10011110;
byte const CHAR_r = B00001010;
byte const CHAR_H = B01101110;
byte const CHAR_h = B00101110;
byte const CHAR_b = B00111110;
byte const CHAR_d = B01111010;
byte const CHAR_DOT = B00000001;
byte const CHAR_SPACE = B00000000;
byte const CHAR_MINUS = B00000010;

byte const ZERO = B11111100;
byte const ONE = B01100000;
byte const TWO = B11011010;
byte const THREE = B11110010;
byte const FOUR = B01100110;
byte const FIVE = B10110110;
byte const SIX = B10111110;
byte const SEVEN = B11100000;
byte const EIGHT = B11111110;
byte const NINE = B11110110;

struct char_2_mask_map
{
	char c;
	byte mask;
};

char_2_mask_map char_2_mask_maps[] =
{
	{'A', CHAR_A},
	{'C', CHAR_C},
	{'O', CHAR_O},
	{'E', CHAR_E},
	{'r', CHAR_r},
	{'H', CHAR_H},
	{'h', CHAR_h},
	{'d', CHAR_d},
	{'b', CHAR_b},
	{'.', CHAR_DOT},
	{' ', CHAR_SPACE},
	{'-', CHAR_MINUS},

	{'0', ZERO},
	{'1', ONE},
	{'2', TWO},
	{'3', THREE},
	{'4', FOUR},
	{'5', FIVE},
	{'6', SIX},
	{'7', SEVEN},
	{'8', EIGHT},
	{'9', NINE}
};
#pragma endregion LED definitions

int pins[] = {9, 13, 4, 6, 7, 10, 3, 5};
int pinDigits[] = {8, 11, 12, A4};

String temperature = "--.-0";
String humidity = "--.-0";

enum DisplayMode { Temperature, Humidity };

DisplayMode displayMode = Temperature;

unsigned long previousSwitchingDisplayModeMillis = 0;
const unsigned long SWITCHING_DISPLAY_MODE_DELAY = 10000;

unsigned long previousMeasureMillis = 0;
const unsigned long MEASURE_DELAY = 5000;

void showMask(byte mask)
{
	const byte segmentsCount = 7;

	for (int i = 0; i < 8; i++)
	{
		digitalWrite(pins[i], !bitRead(mask, segmentsCount - i));
	}
}

void showMask(byte digitNumber, const byte mask)
{
	showMask(mask);
	digitalWrite(pinDigits[digitNumber], HIGH);
	delay(1);
	digitalWrite(pinDigits[digitNumber], LOW);
}

byte getMask(const char c, const boolean withDot)
{
	for (auto i = 0; i < sizeof(char_2_mask_maps) / sizeof(*char_2_mask_maps); i++)
	{
		if (c == char_2_mask_maps[i].c)
		{
			byte result = char_2_mask_maps[i].mask;
			if (withDot)
			{
				result |= 1;
			}

			return result;
		}
	}

	return CHAR_SPACE;
}

byte getMask(const char c)
{
	return getMask(c, false);
}

void displayText(String text)
{
	int digitNumber = 0;

	for (byte i = 0; i < text.length(); i++)
	{
		if (i < text.length() - 1)
		{
			if (text.charAt(i) != '.' && text.charAt(i + 1) == '.')
			{
				showMask(digitNumber, getMask(text[i], true));
				continue;
			}
		}
		showMask(digitNumber, getMask(text[i]));
		digitNumber++;
	}
}

String replaceLastZero(const String s, const char c)
{
	String result = s;
	const unsigned max_chars_number = 6;
	if (s.length() <= max_chars_number && s.endsWith("0"))
	{
		result.setCharAt(s.length() - 1, c);
	}

	return result;
}

void setup()
{
	Serial.begin(9600);

	// Initialize device.
	dht.begin();

	for (int i = 0; i < SEGMENTS_COUNT; i++)
	{
		pinMode(pins[i], OUTPUT);
		digitalWrite(pinDigits[i], LOW);
	}

	for (int i = 0; i < NUMBER_OF_DIGITS; i++)
	{
		pinMode(pinDigits[i], OUTPUT);
		digitalWrite(pinDigits[i], LOW);
	}
}

void loop()
{
	unsigned long currentMilis = millis();

	if (currentMilis - previousMeasureMillis > MEASURE_DELAY)
	{
		// Get temperature event and print its value.
		sensors_event_t event;
		dht.temperature().getEvent(&event);
		if (isnan(event.temperature))
		{
			Serial.println(F("Error reading temperature!"));
			temperature = "Err.C";
		}
		else
		{
			Serial.print(F("Temperature: "));
			Serial.print(event.temperature);
			Serial.print("°C");
			temperature = String(event.temperature);
		}
		dht.humidity().getEvent(&event);
		if (isnan(event.relative_humidity))
		{
			Serial.println(F("Error reading humidity!"));
			humidity = "Err.H";
		}
		else
		{
			Serial.print(F("    Humidity: "));
			Serial.print(event.relative_humidity);
			Serial.print("%");
			humidity = String(event.relative_humidity);
		}

		previousMeasureMillis += MEASURE_DELAY;
	}

	if (millis() - previousSwitchingDisplayModeMillis >= SWITCHING_DISPLAY_MODE_DELAY)
	{
		displayMode = (displayMode == Temperature ? Humidity : Temperature);

		previousSwitchingDisplayModeMillis += SWITCHING_DISPLAY_MODE_DELAY;
	}

	switch (displayMode)
	{
	case Temperature:
		displayText(replaceLastZero(temperature, 'C'));
		break;
	case Humidity:
		displayText(replaceLastZero(humidity, 'H'));
	default: ;
	}
}
