//�����x�Z���T���W���[�����g���ĉ��x�A���x�𑪒肷��X�P�b�`
#include "Arduino.h"
#include "SPI.h"
#include "DHT.h"

#define DHT_Pin	(2)		//DHT11��DATA�s�����`
#define DHT_Type	(DHT11) //�Z���T�̌^�Ԓ�`�@DHT11,DHT22�Ȃ�

DHT dht(DHT_Pin, DHT_Type); //�Z���T������

//�v���l
float humidity = 0.0f; //���x
float temperature = 0.0f; 	//���x

void setup() {
	Serial.begin(9600);
	Serial.println("DHT11 sample"); //��ʂɕ\��
	dht.begin(); //�����x�Z���T�[�J�n
}

void loop() {
	delay(5000); //���m�ȃf�[�^�擾�̂��߂�5�b�ȏ�Ԋu��u��

	//�f�[�^�ǂݏo��
	humidity = dht.readHumidity(); //���x�̓ǂݏo��
	temperature = dht.readTemperature(); //���x�̓ǂݏo��(�ێ�)

	//�ǂݎ��l�̊m�F
	if (isnan(humidity) || isnan(temperature)) {
		Serial.println("Read failure!");
		return;
	}

	/* �ȉ��ǂݎ��l�̕\�� */
	Serial.print("���x: ");
	Serial.print(humidity);
	Serial.println("[%]");
	Serial.print("���x: ");
	Serial.print(temperature);
	Serial.println("[��]");
}
