#define DPTF_CPU_PASSIVE	80
#define DPTF_CPU_CRITICAL	90

#define DPTF_TSR0_SENSOR_ID	1
#define DPTF_TSR0_SENSOR_NAME	"TMP432_Internal"
#define DPTF_TSR0_PASSIVE	53
#define DPTF_TSR0_CRITICAL	75

#define DPTF_TSR1_SENSOR_ID	2
#define DPTF_TSR1_SENSOR_NAME	"TMP432_Power_top"
#define DPTF_TSR1_PASSIVE	60
#define DPTF_TSR1_CRITICAL	80

#define DPTF_TSR2_SENSOR_ID	3
#define DPTF_TSR2_SENSOR_NAME	"TMP432_CPU_bottom"
#define DPTF_TSR2_PASSIVE	80
#define DPTF_TSR2_CRITICAL	100

#define DPTF_ENABLE_CHARGER

/* Charger performance states, board-specific values from charger and EC */
Name (CHPS, Package () {
	Package () { 0, 0, 0, 0, 255, 0x7d0, "mA", 0 }, /* 2.0A */
	Package () { 0, 0, 0, 0, 24, 0x6a4, "mA", 0 },  /* 1.7A */
	Package () { 0, 0, 0, 0, 16, 0x578, "mA", 0 },  /* 1.4A */
	Package () { 0, 0, 0, 0, 8, 0x3e8, "mA", 0 },   /* 1.0A */
	Package () { 0, 0, 0, 0, 0, 0x258, "mA", 0 },   /* 0.6A */
})

/* Mainboard specific _PDL is 1GHz */
Name (MPDL, 8)

Name (DTRT, Package () {
	/* CPU Throttle Effect on CPU */
	Package () { \_SB.DPTF.TCPU, \_SB.DPTF.TCPU, 100, 200, 0, 0, 0, 0 },

	/* CPU Effect on Temp Sensor 0 */
	Package () { \_SB.DPTF.TCPU, \_SB.DPTF.TSR0, 100, 150, 0, 0, 0, 0 },

#ifdef DPTF_ENABLE_CHARGER
	/* Charger Effect on Temp Sensor 1 */
	Package () { \_SB.DPTF.TCHG, \_SB.DPTF.TSR1, 200, 300, 0, 0, 0, 0 },

	/* Charger Effect on Temp Sensor 2 */
	Package () { \_SB.DPTF.TCHG, \_SB.DPTF.TSR2, 100, 600, 0, 0, 0, 0 },
#endif
})

Name (MPPC, Package ()
{
	0x2,		// Revision
	Package () {	// Power Limit 1
		0,	// PowerLimitIndex, 0 for Power Limit 1
		2100,	// PowerLimitMinimum
		6300,	// PowerLimitMaximum
		1000,	// TimeWindowMinimum
		1000,	// TimeWindowMaximum
		200	// StepSize
	},
	Package () {	// Power Limit 2
		1,	// PowerLimitIndex, 1 for Power Limit 2
		8000,	// PowerLimitMinimum
		8000,	// PowerLimitMaximum
		1000,	// TimeWindowMinimum
		1000,	// TimeWindowMaximum
		1000	// StepSize
	}
})

/* Include Baytrail DPTF */
#include <soc/intel/baytrail/acpi/dptf/dptf.asl>
