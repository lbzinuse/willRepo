/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * openobd
 * Copyright (C) Simon Booth 2010 <simesb@users.sourceforge.net>
 *
 * openobd is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * openobd is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _OBDBASE_H_
#define _OBDBASE_H_

#include <vector>
#include "ctb-0.15/ctb.h"
#include "logPanel.h"

using namespace std;
using namespace ctb;

#define PID_DTC_STATUS		0x0101
#define PID_DTCFRZF         0x0102
#define PID_FUELSYS         0x0103  ///< Fuel system status
#define PID_LOAD_PCT        0x0104  ///< Calculated load value
#define PID_ECT             0x0105  ///< Engine Coolant Temperature
#define PID_BANK1_STFT		0x0106
#define PID_BANK1_LTFT		0x0107
#define PID_BANK2_STFT		0x0108
#define PID_BANK2_LTFT		0x0109
#define PID_FRP             0x010A  ///< Fuel rail pressure (Gauge)
#define PID_MAP             0x010B  ///< Intake manifold absolute pressure
#define PID_RPM             0x010C  ///< Engine RPM
#define PID_VSS             0x010D  ///< Vehicle speed sensor
#define PID_SPARKADV        0x010E  ///< Timing advance for #1 cylinder
#define PID_IAT             0x010F  ///< Intake Air Temperature
#define PID_MAF             0x0110  ///< Air Flow Rate from Mass Air Flow Sensor
#define PID_TP              0x0111  ///< Absolute Throttle Position
#define PID_AIR_STAT        0x0112  ///< Commanded Secondary Air Status
#define PID_O2SLOC          0x0113  ///< Location of Oxygen sensors
#define PID_O2S11           0x0114
#define PID_O2S12           0x0115
#define PID_O2S13           0x0116
#define PID_O2S14           0x0117
#define PID_O2S21           0x0118
#define PID_O2S22           0x0119
#define PID_O2S23           0x011A
#define PID_O2S24           0x011B
#define PID_OBDSUP          0x011C  ///< OBD supported by vehicle
#define PID_PTO_STAT        0x011E  ///< Auxilliary Input Status
#define PID_RUNTM           0x011F  ///< Engine runtime
#define PID_MIL_DIST		0x0121  ///< Distance travelled with MIL illuminated
#define PID_FRP_REL         0x0122  ///< Fuel rail pressure (relative to manifold)
#define PID_FRP_ATMO        0x0123  ///< Fuel rail pressure (relative to atmosphere)
#define PID_EGR_PCT         0x012C  ///< Commanded EGR
#define PID_EGR_ERR         0x012D  ///< EGR Error
#define PID_EVAP_PCT        0x012E  ///< Commanded Evaporative Purge
#define PID_FLI             0x012F  ///< Fuel level input
#define PID_WARM_UPS        0x0130  ///< Warm-ups since trouble codes cleared
#define PID_CLR_DIST        0x0131  ///< Distance since trouble codes cleared
#define PID_EVAP_VP         0x0132  ///< Evap system vapour pressure
#define PID_BARO            0x0133  ///< Barometric pressure
#define PID_CATEMP11        0x013C  ///< Catalyst temperature Bank 1, Sensor 1
#define PID_CATEMP21        0x013D  ///< Catalyst temperature Bank 2, Sensor 1
#define PID_CATEMP12        0x013E  ///< Catalyst temperature Bank 1, Sensor 2
#define PID_CATEMP22        0x013F  ///< Catalyst temperature Bank 2, Sensor 2
#define PID_VPWR            0x0142  ///< Control module voltage
#define PID_LOAD_ABS        0x0143  ///< Absolute load value
#define PID_EQ_RAT          0x0144  ///< Commanded equivalence ratio
#define PID_TP_R            0x0145  ///< Relative throttle position
#define PID_AAT             0x0146  ///< Ambient air temp
#define PID_TP_B            0x0147  ///< Absolute throttle position B
#define PID_TP_C            0x0148  ///< Absolute throttle position C
#define PID_APP_D           0x0149  ///< Accelerator pedal position D
#define PID_APP_E           0x014A  ///< Accelerator pedal position E
#define PID_APP_F           0x014B  ///< Accelerator pedal position F
#define PID_TAC_PCT         0x014C  ///< Commanded throttle actuator control
#define PID_MIL_TIME		0x014D  ///< Time run with MIL lit
#define PID_CLR_TIME        0x014E  ///< Time run since MIL cleared
#define PID_FUEL_TYP        0x0151  ///< Type of fuel in use
#define PID_ALCH_PCT        0x0152  ///< Alcohol fuel percentage
#define PID_EVAP_VPA        0x0153  ///< Absolute Evap System Vapour Pressure
#define PID_EVAP_OTHER      0x0154
#define PID_FRP_ABS         0x0159  ///< Fuel rail pressure (absolute)
#define PID_APP_R           0x015A  ///< Relative Accelerator Pedal Position

#define PID_VIN				0x0902  ///< Vehicle Identification Number

class obdbase
{
public:

    struct pidInfo {
        int pid_flag;
        double resultMain;
        double resultSecondary;
        wxString resultString;
    };

    enum pidFlag {
        PID_FLAG_SINGLE,
        PID_FLAG_DOUBLE,
        PID_FLAG_STRING
    };

    obdbase ();
	obdbase (const wxString& serialPort);
	~obdbase ();

	// admin functions
	virtual wxString obdDeviceIdentify ();
	virtual void obd_device_soft_reset () {};
	virtual void obdInitFast () {};
	virtual void obdInitSlow () {};
	virtual void obdProtocolSet () {};
	virtual wxString obdProtocolGet ();
	virtual void obdDeviceDisconnect();
	virtual void obd_use_checksums (bool use);
	virtual void obd_use_imperial (bool use);
	virtual bool obd_is_imperial ();
	bool obd_is_connected();
	void obd_set_logger (logPanel* log);

	// error code functions
	virtual int obd_mil_status();
	virtual wxArrayString obd_mil_error_codes ();
	virtual bool obd_clear_dtc ();

	// PID functions
	virtual bool obd_pid_get_raw (int pid, int tokens[], int toksize);
	virtual bool obd_pid_value(int pid, obdbase::pidInfo* result);
    virtual void obdSupportedPids(int mode, std::vector<int>& pids);

protected:
	ctb::SerialPort* port;
	logPanel* logger;
	bool logExtra;

	bool useChecksum;
	bool useImperial;

	// connection functions
	virtual void obdDeviceConnect (const wxString& SerialPort);
	virtual bool obdWrite(const wxString& command, int count);
	virtual wxString obdRead();

	// checksum functions
	void obdChecksumCalculate ();
	bool obdCheckumValidate ();
	void obdChecksumAppend ();

private:

    // number of error codes retreived at last attempt.
    int lastErrorCount;

    // conversion functions
    void convertToImperial(int pid, obdbase::pidInfo* result);
	int convert_c_to_f (int centigrade);
	int convert_kph_to_mph (int kph);
	double convert_kpa_to_psi(double kpa);
	double convert_kpa_to_inhg(double kpa);
	double convert_kpa_to_inh20(double kpa);
    double convert_pa_to_inh20(double pa);
	double convert_pa_to_inhg(double pa);
	double convert_gs_to_lbs(double gs);
	double convert_gs_to_lbmin(double gs);

    // functions to decode byte-encode PIDS
    wxString obd_pid_air_stat(int encByte);
    wxString obd_pid_obd_supported(int encByte);
    wxString obd_pid_fuel_type(int encByte);
    wxString obd_pid_vin(int tokens[]);
};

#endif // _OBDBASE_H_
