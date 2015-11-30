/*
 * rt5640.h  --  RT5640 Soc Audio driver platform data
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _RT5640_PDATA_H
#define _RT5640_PDATA_H

#define RT5640_DRES_400R 0
#define RT5640_DRES_200R 1
#define RT5640_DRES_600R 2
#define RT5640_DRES_150R 3
#define RT5640_DRES_MAX  3

struct rt5640_data {
	bool capless;  /* Headphone outputs configured in capless mode */

	int dres;  /* Discharge resistance for headphone outputs */
};

#endif
