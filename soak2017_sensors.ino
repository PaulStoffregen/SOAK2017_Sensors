
const int en_pin = 2;
const int m0_pin = 7;
const int m1_pin = 8;
const int m2_pin = 9;
elapsedMicros usec;
const int col2analog[6] = {A6, A5, A4, A7, A8, A9};

const int usecmax = 16666;
const int usecdead = 10;
const int nmax = 4;

void setup()
{
	pinMode(en_pin, OUTPUT);
	digitalWriteFast(en_pin, LOW);
	analogReadAveraging(1);
	pinMode(m0_pin, OUTPUT);
	pinMode(m1_pin, OUTPUT);
	pinMode(m2_pin, OUTPUT);
	usec = 0;
}

void loop()
{
 	unsigned int dark[8][6], light[8][6];
	unsigned int n, row, col, count;

	digitalWriteFast(en_pin, LOW);
	memset(dark, 0, sizeof(dark));
	count = 0;
	while (usec < usecmax) {
		for (row=0; row < 8; row++) {
			digitalWriteFast(m0_pin, ((row & 1) ? HIGH : LOW));
			digitalWriteFast(m1_pin, ((row & 2) ? HIGH : LOW));
			digitalWriteFast(m2_pin, ((row & 4) ? HIGH : LOW));
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
			digitalWriteFast(m0_pin, ((row & 1) ? HIGH : LOW));
			digitalWriteFast(m1_pin, ((row & 2) ? HIGH : LOW));
			digitalWriteFast(m2_pin, ((row & 4) ? HIGH : LOW));
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

	Serial.print(dark[0][0]);
	Serial.print("   ");
	Serial.print(light[0][0]);
	Serial.print("   ");
	Serial.print((int)(light[0][0] - dark[0][0]));
	Serial.println();


}
