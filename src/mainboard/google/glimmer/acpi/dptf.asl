#define DPTF_CPU_PASSIVE	60
#define DPTF_CPU_CRITICAL	70

#define DPTF_TSR0_SENSOR_ID	1
#define DPTF_TSR0_SENSOR_NAME	"TMP432_Power_Bottom"
#define DPTF_TSR0_PASSIVE	60
#define DPTF_TSR0_CRITICAL	70

#define DPTF_TSR1_SENSOR_ID	2
#define DPTF_TSR1_SENSOR_NAME	"TMP432_RAM_bottom"
#define DPTF_TSR1_PASSIVE	60
#define DPTF_TSR1_CRITICAL	70

#define DPTF_TSR2_SENSOR_ID	3
#define DPTF_TSR2_SENSOR_NAME	"TMP432_CPU_bottom"
#define DPTF_TSR2_PASSIVE	60
#define DPTF_TSR2_CRITICAL	70

#undef DPTF_ENABLE_CHARGER

Name (DTRT, Package () {
	/* CPU Throttle Effect on CPU */
	Package () { \_SB.DPTF.TCPU, \_SB.DPTF.TCPU, 100, 50, 0, 0, 0, 0 },

	/* CPU Effect on Temp Sensor 0 */
	Package () { \_SB.DPTF.TCPU, \_SB.DPTF.TSR0, 100, 50, 0, 0, 0, 0 },

#ifdef DPTF_ENABLE_CHARGER
	/* Charger Effect on Temp Sensor 1 */
	Package () { \_SB.DPTF.TCHG, \_SB.DPTF.TSR1, 200, 50, 0, 0, 0, 0 },
#endif

	/* CPU Effect on Temp Sensor 1 */
	Package () { \_SB.DPTF.TCPU, \_SB.DPTF.TSR1, 100, 50, 0, 0, 0, 0 },

	/* CPU Effect on Temp Sensor 2 */
	Package () { \_SB.DPTF.TCPU, \_SB.DPTF.TSR2, 100, 50, 0, 0, 0, 0 },
})

/* Include Baytrail DPTF */
#include <soc/intel/baytrail/acpi/dptf/dptf.asl>
