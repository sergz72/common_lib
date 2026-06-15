#include <board.h>
#include <pir_sensor.h>
#include <string.h>

#ifdef PIR_SENSOR_FILTER_CRS
volatile short pir_filter_output;
#endif
static short pir_adc_samples[PIR_SENSOR_AVERAGING_FILTER_SAMPLES_COUNT];
static long pir_adc_samples_sum;
static long pir_adc_samples_acc;
volatile short pir_adc_samples_avg;
static unsigned int pir_adc_samples_idx;

void pir_sensor_init(void)
{
  memset(pir_adc_samples, 0, sizeof(pir_adc_samples));
  pir_filter_output = 0;
  pir_adc_samples_sum = 0;
  pir_adc_samples_acc = 0;
  pir_adc_samples_avg = 0;
  pir_adc_samples_idx = 0;
}

void pir_sensor_adc_handler(short value)
{
#ifdef PIR_SENSOR_FILTER_CRS
  pir_adc_samples_acc = pir_adc_samples_acc + value - (pir_adc_samples_acc >> PIR_SENSOR_FILTER_CRS);
  pir_filter_output = pir_adc_samples_acc >> PIR_SENSOR_FILTER_CRS;
#else
  unsigned short pir_filter_output = value;
#endif

  pir_adc_samples_sum -= pir_adc_samples[pir_adc_samples_idx];
  pir_adc_samples[pir_adc_samples_idx] = pir_filter_output;
  if (pir_adc_samples_idx >= PIR_SENSOR_AVERAGING_FILTER_SAMPLES_COUNT - 1)
    pir_adc_samples_idx = 0;
  else
    pir_adc_samples_idx++;
  pir_adc_samples_sum += pir_filter_output;
  pir_adc_samples_avg = (short)(pir_adc_samples_sum / PIR_SENSOR_AVERAGING_FILTER_SAMPLES_COUNT);

  if ((pir_filter_output > pir_adc_samples_avg + PIR_SENSOR_FILTER_THRESHOLD) ||
    (pir_filter_output < pir_adc_samples_avg - PIR_SENSOR_FILTER_THRESHOLD))
    pir_motion_detected();
}