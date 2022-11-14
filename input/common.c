#include "input/common.h"
#include <math.h>
#include <string.h>

int write_to_cava_input_buffers(int16_t samples, unsigned char *buf, void *data) {
    if (samples == 0)
        return 0;
    struct audio_data *audio = (struct audio_data *)data;
    int16_t buf16;
    uint16_t ubuf16;
    pthread_mutex_lock(&audio->lock);
    int bytes_per_sample = audio->format / 8;
    if (audio->samples_counter + samples > audio->cava_buffer_size) {
        // buffer overflow, discard what ever is in the buffer and start over
        for (uint16_t n = 0; n < audio->cava_buffer_size; n++) {
            audio->cava_in[n] = 0;
        }
        audio->samples_counter = 0;
    }
    int n = 0;
    for (uint16_t i = 0; i < samples; i++) {
        switch (bytes_per_sample) {
        case 1:
            ;
            int8_t *buf8 = &buf[n];
            audio->cava_in[i + audio->samples_counter] = *buf8 * UCHAR_MAX;
            break;
        case 3:
        case 4:
            ;
            if (audio->IEEE_FLOAT) {
                float *ieee_float = &buf[n];
                audio->cava_in[i + audio->samples_counter] = *ieee_float * USHRT_MAX;
            } else {
                int32_t *buf32 = &buf[n];
                audio->cava_in[i + audio->samples_counter] = (double)*buf32 / USHRT_MAX;
            }
            break;
        default://we default to 16
            ;
            int16_t *buf16 = &buf[n];
            audio->cava_in[i + audio->samples_counter] = *buf16;
            break;
        }
        n += bytes_per_sample;
    }
    audio->samples_counter += samples;
    pthread_mutex_unlock(&audio->lock);
    return 0;
}

void reset_output_buffers(struct audio_data *data) {
    struct audio_data *audio = (struct audio_data *)data;
    pthread_mutex_lock(&audio->lock);
    for (uint16_t n = 0; n < audio->cava_buffer_size; n++) {
        audio->cava_in[n] = 0;
    }
    pthread_mutex_unlock(&audio->lock);
}