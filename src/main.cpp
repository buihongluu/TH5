#include <Wire.h>
#include <BH1750.h>
#include <edge-impulse-sdk/classifier/ei_run_classifier.h>

// Số lượng nhãn bạn huấn luyện (cập nhật đúng nếu khác)
#define NUM_CLASSES 3

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

BH1750 lightMeter;

void setup() {
    Serial.begin(115200);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println("BH1750 ready");
    } else {
        Serial.println("BH1750 not found");
        while (1);
    }
}

void loop() {
    float lux = lightMeter.readLightLevel();

    if (lux < 0) {
        Serial.println("Lỗi đọc cảm biến");
        delay(500);
        return;
    }

    Serial.print("Lux: ");
    Serial.println(lux, 2);

    // Chuẩn bị dữ liệu cho Edge Impulse
    signal_t signal;
    static float features[] = { lux };

    signal.total_length = 1;
    signal.get_data = [](size_t offset, size_t length, float *out_ptr) -> int {
        out_ptr[0] = features[0];
        return 0;
    };

    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

    if (res != EI_IMPULSE_OK) {
        Serial.print("Classification failed: ");
        Serial.println(res);
        return;
    }

    // Duyệt qua các kết quả phân loại
    for (size_t ix = 0; ix < NUM_CLASSES; ix++) {
        ei_impulse_result_classification_t c = result.classification[ix];
        Serial.print("Label: ");
        Serial.print(c.label);
        Serial.print(" => Confidence: ");
        Serial.println(c.value, 3);
    }

    Serial.println("--------------------------");
    delay(1000);
}
