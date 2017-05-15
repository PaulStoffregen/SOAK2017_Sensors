
const int en_pin = 2;
const int m0_pin = 7;
const int m1_pin = 8;
const int m2_pin = 9;
elapsedMicros usec;
const int col2analog[6] = {A6, A5, A4, A7, A8, A9};

const int usecmax = 16666 * 1;
const int usecdead = 10;
const int nmax = 4;

uint8_t scale_table[1780];
unsigned int baseline[8][6];
unsigned int numrun=0;

void setup()
{
	Serial1.begin(19200);
	pinMode(en_pin, OUTPUT);
	digitalWriteFast(en_pin, LOW);
	analogReadAveraging(1);
	pinMode(m0_pin, OUTPUT);
	pinMode(m1_pin, OUTPUT);
	pinMode(m2_pin, OUTPUT);
	usec = 0;
	//while (!Serial);
	for (unsigned int i=0; i < sizeof(scale_table); i++) {
		//scale_table[i] = sqrtf(i * 35) + 0.5;
		scale_table[i] = sinf((float)i / (float)sizeof(scale_table) * 1.570796327) * 250;
		//Serial.printf("%d -> %d\n", i, scale_table[i]);
	}
	for (unsigned int row=0; row < 8; row++) {
		// start with a high baseline and let the learning
		// quickly lower it.
		for (unsigned int col=0; col < 6; col++) {
			baseline[row][col] = 6000000;
		}
	}
}

void loop()
{
 	unsigned int dark[8][6], light[8][6], diff[8][6];
	unsigned int n, row, col, count;

	digitalWriteFast(en_pin, LOW);
	memset(dark, 0, sizeof(dark));
	count = 0;
	while (usec < usecmax) {
		for (row=0; row < 8; row++) {
			unsigned int mux = (row < 4) ? 3 - row : row;
			digitalWriteFast(m0_pin, ((mux & 1) ? HIGH : LOW));
			digitalWriteFast(m1_pin, ((mux & 2) ? HIGH : LOW));
			digitalWriteFast(m2_pin, ((mux & 4) ? HIGH : LOW));
			delayMicroseconds(usecdead);
			for (n=0; n < nmax; n++) {
				for (col=0; col < 6; col++) {
					dark[row][col] += analogRead(col2analog[col]) << 4;
				}
			}
		}
		count += nmax;
	}
	for (row=0; row < 8; row++) {
		for (col=0; col < 6; col++) {
			dark[row][col] /= count;
		}
	}
	usec -= usecmax;
	digitalWriteFast(en_pin, HIGH);
	memset(light, 0, sizeof(light));
	count = 0;
	while (usec < usecmax) {
		for (row=0; row < 8; row++) {
			unsigned int mux = (row < 4) ? 3 - row : row;
			digitalWriteFast(m0_pin, ((mux & 1) ? HIGH : LOW));
			digitalWriteFast(m1_pin, ((mux & 2) ? HIGH : LOW));
			digitalWriteFast(m2_pin, ((mux & 4) ? HIGH : LOW));
			delayMicroseconds(usecdead);
			for (n=0; n < nmax; n++) {
				for (col=0; col < 6; col++) {
					light[row][col] += analogRead(col2analog[col]) << 4;
				}
			}
		}
		count += nmax;
	}
	for (row=0; row < 8; row++) {
		for (col=0; col < 6; col++) {
			light[row][col] /= count;
		}
	}
	usec -= usecmax;

	for (row=0; row < 8; row++) {
		for (col=0; col < 6; col++) {
			int x = (int)(light[row][col] - dark[row][col]);
			if (x < 0) x = 0;
			diff[row][col] = x;
			//Serial.printf("%4d ", x);
		}
	}
	if (++numrun > 20) {
		// ignore the first 20 readings (tends to be garbage)
		// then normal running, update and subtract baseline
		Serial1.write(252);
		for (row=0; row < 8; row++) {
			for (col=0; col < 6; col++) {
				int change = diff[row][col] * 4096 - baseline[row][col];
				if (change >= 0) {
					baseline[row][col] += change / 60; // dec speed
					diff[row][col] -= baseline[row][col] / 4096;
					if (diff[row][col] > 12) {
						diff[row][col] -= 12; // min threshold
					} else {
						diff[row][col] = 0;
					}
				} else {
					baseline[row][col] += change / 3; // inc speed
					diff[row][col] = 0;
				}
				if (diff[row][col] < sizeof(scale_table)) {
					// non-linear scaling
					diff[row][col] = scale_table[diff[row][col]];
					if (diff[row][col] > 250) diff[row][col] = 250;
				} else {
					diff[row][col] = 250; // max value
				}
				//Serial.printf("%4d ", diff[row][col]);
				Serial1.write(diff[row][col]);
			}
		}
	}
	//Serial.print("   -   ");
	//Serial.printf("%5d ", (int)(light[7][0] - dark[7][0]));
	//Serial.printf("%5d ", baseline[7][0] / 4096);
	//Serial.printf("%5d ", baseline[7][0]);

	//Serial.print(dark[0][0]);
	//Serial.print("   ");
	//Serial.print(light[0][0]);
	//Serial.print("   ");
	//Serial.print((int)(light[0][0] - dark[0][0]));
	//Serial.println();
}

