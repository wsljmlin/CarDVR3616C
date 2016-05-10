#ifndef	__HI_GPIO_H__
#define	__HI_GPIO_H__
#define	IRCUT_DEV	"/dev/MIX_DEV"
#define HI_IRCUT_REG_CLEAR	_IOW('I', 0,  int)
#define HI_IRCUT_REG_SET	_IOW('I', 1,  int)
#define HI_WIFI_REG_CLEAR	_IOW('W', 0,  int)
#define HI_WIFI_REG_SET		_IOW('W', 1,  int)
#endif
