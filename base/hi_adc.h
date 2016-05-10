#ifndef __HI_ADC_H__
#define __HI_ADC_H__

/* io ctrl command */
#define	HI_ADC_IOC_MAGIC	'A'
#define    HI_ADC_IOC_SET_SAMPLES    _IOW(HI_ADC_IOC_MAGIC, 1, int)
#define    HI_ADC_IOC_GET_SAMPLES    _IOR(HI_ADC_IOC_MAGIC, 2, int)
#define    HI_ADC_IOC_SET_THROD    _IOW(HI_ADC_IOC_MAGIC, 3, int)
#define    HI_ADC_IOC_GET_THROD    _IOR(HI_ADC_IOC_MAGIC, 4, int)
#define    HI_ADC_IOC_SET_CH    _IOW(HI_ADC_IOC_MAGIC, 5, int)
#define    HI_ADC_IOC_GET_CH    _IOR(HI_ADC_IOC_MAGIC, 6, int)

#endif