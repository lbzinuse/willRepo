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

/// \class obdbase
/// \brief A base class for interacting with an OBD-II interface device.

#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/wx.h>
    #include <wx/string.h>
#endif

#include <cstring>
#include <fcntl.h>

#include <wx/tokenzr.h>

#include "obdbase.h"
#include "logPanel.h"
#include "ctb-0.15/ctb.h"

using namespace std;
using namespace ctb;

obdbase::obdbase ()
{
    // create a new serial port object
	port = new ctb::SerialPort();

	// setup the defaults
	this->obd_use_checksums(true);
	this->obd_use_imperial(false);

	this->logger = NULL;
	this->logExtra = true;
}

obdbase::obdbase (const wxString& SerialPort)
{
    // create a new serial port object
	port = new ctb::SerialPort();
	// and connect it to the chosen port
    this->obdDeviceConnect(SerialPort);

	// setup the defaults
	this->obd_use_checksums(true);
	this->obd_use_imperial(false);

	this->logger = NULL;
	this->logExtra = true;
	this->lastErrorCount = 0;
}

obdbase::~obdbase ()
{
    this->obdDeviceDisconnect();
    delete port;
}

void obdbase::obdDeviceConnect (const wxString& SerialPort)
{
    int baudrate = 9600;

    // open the serial port
    port->Open(SerialPort.mb_str(wxConvUTF8), baudrate);
}

void obdbase::obdDeviceDisconnect ()
{
    // close the port
    port->Close();
}

void obdbase::obd_use_checksums (bool use)
{
	this->useChecksum = use;
}

void obdbase::obd_use_imperial (bool use)
{
	this->useImperial = use;
}

bool obdbase::obd_is_imperial ()
{
	return this->useImperial;
}

void obdbase::obd_set_logger (logPanel* log)
{
	this->logger = log;
}

int obdbase::obd_mil_status()
{
	int result = -1;
	int toks[10];
	wxString msg;

	// write to log if necessary
	if (logger) {
		msg = _("Requesting MIL Status\n");
		logger->appendLog(msg, logPanel::LOG_OUT);
	}

	// return -1 if we haven't got a well formed response
	if (this->obd_pid_get_raw(PID_DTC_STATUS, toks, sizeof(toks))) {

		// get the value of the '3rd byte' and apply conversion formula
		if (toks[2] > 0x80) {
			result = toks[2] - 0x80;
		} else {
			result = 0;
		}

		// write to log if necessary
		if (logger) {
			msg.Printf(_("Received MIL Response: %d.\n"), result);
			logger->appendLog(msg, logPanel::LOG_IN);
		}
	}

    // update the stored number of error codes
    this->lastErrorCount = result;

    // return the number of codes
	return result;
}

/// \brief Retrieve the error codes from the interface
///
/// A variable number of error codes may be returned.  Each code must
/// be decoded from the returned 'bytes'.  Will return an empty array
/// if the MIL status has not been checked first
///
/// \see obd_mil_status()
/// \return An array of strings with the error codes
wxArrayString obdbase::obd_mil_error_codes ()
{
    int toks[30];
    int byteOne;
    int byteTwo;
	wxString msg;
	wxArrayString errors;
	wxString currError;

	// bail out early if no error codes to be retrieved.
	if (this->lastErrorCount == 0) {
	    // write to log if necessary
        if (logger) {
            msg = _("No MIL Codes have been reported\n");
            logger->appendLog(msg, logPanel::LOG_ERROR);
        }
	} else {
        // write to log if necessary
        if (logger) {
            msg = _("Requesting MIL Codes\n");
            logger->appendLog(msg, logPanel::LOG_OUT);
        }

        if (this->obd_pid_get_raw(0x03, toks, sizeof(toks))) {

            // for each code, convert to string
            for (int i = 0; i < this->lastErrorCount; i++) {
                // make sure we have an empty string
                currError.Clear();

                // work out the first and second 'byte'
                int frame = i / 3;
                byteOne = toks[i+i+1+frame];
                byteTwo = toks[i+i+2+frame];

                // char 1
                // defined by first 2 bits of first byte
                int x = (byteOne & 0xC0) >> 6;

                // select the correct char based upon the value of
                // the first two bits
                switch (x) {
                    case 1:
                        currError = _T("C");
                        break;
                    case 2:
                        currError = _T("B");
                        break;
                    case 3:
                        currError = _T("U");
                        break;
                    default:
                        currError = _T("P");
                }

                // char 2
                // defined by 3rd & 4th bits of first byte
                currError += wxString::Format(_T("%d"), (byteOne & 0x30) >> 4);

                // char 3
                // defined by 5th, 6th, 7th, and 8th bit of first byte
                currError += wxString::Format(_T("%d"), byteOne & 0x0F);

                // chars 4 & 5
                // defined by second byte
                currError += wxString::Format(_T("%.2d"), byteTwo);

                // now we have a code, add string to array
                errors.Add(currError);
            }
        }
	}

    // return the array of errors
	return errors;
}

/// \brief Ask the ECU to clear the DTCs and switch off the MIL
///
/// The MIL (Malfunction Indicator Lamp) may be illuminated due to one
/// or more error codes which are stored by the ECU.  This function
/// asks the ECU to clear these codes and switch off the lamp.
///
/// \return True if the ECU has reponded and cleared the codes.
bool obdbase::obd_clear_dtc ()
{
	bool result = false;
	int toks[3];
	wxString msg;

	// write to log if necessary
	if (logger) {
		msg = _("Requesting clearing of DTC's\n");
		logger->appendLog(msg, logPanel::LOG_OUT);
	}

	// return -1 if we haven't got a well formed response
	if (this->obd_pid_get_raw(0x04, toks, sizeof(toks))) {
		result = true;

		// write to log if necessary
		if (logger) {
			msg = _("DTC's successfully cleared\n");
			logger->appendLog(msg, logPanel::LOG_IN);
		}
	} else {
		// write to log if necessary
		if (logger) {
			msg = _("Could not clear DTC's\n");
			logger->appendLog(msg, logPanel::LOG_ERROR);
		}
	}

	return result;
}

bool obdbase::obdWrite(const wxString& command, int count)
{
	int len;
	char cstring[256] = "";
	bool retval = false;

	strncat(cstring, (const char*)command.mb_str(wxConvUTF8), command.Length());

	// end the command correctly with a CR
	len = strlen(cstring);
	cstring[len] = 0x0d;
	cstring[len+1] = 0x00;

	if (port->Write(cstring, strlen(cstring)) == (count + 1)) {
		retval = true;
	}

	return retval;
}

wxString obdbase::obdRead()
{
	wxString result;
	char * buff = NULL;
	char * p = (char *)">";
	int read;
	size_t size;

	read = port->ReadUntilEOS(buff, &size, p, 5000, 0);

	// strip out non printing chars
	for (int i = 0; i < sizeof(buff); i++) {
		if (iscntrl(buff[i]))
			buff[i] = 0x20;
	}

	// set up the return string
	result = wxString::From8BitData(buff);
	result = result.Strip(wxString::both);

	delete buff;

	return result;
}

void obdbase::obdChecksumCalculate ()
{
	//TODO Calculate checksum from tokens
}

bool obdbase::obdCheckumValidate ()
{
	//TODO Validate checksum against calculation
	return true;
}

void obdbase::obdChecksumAppend ()
{
}

wxString obdbase::obdDeviceIdentify()
{
    wxString result = wxT("Superclass for devices");
	return result;
}

wxString obdbase::obdProtocolGet()
{
	wxString result = wxT("Superclass for devices");
	return result;
}

bool obdbase::obd_pid_get_raw(int pid, int tokens[], int toksize)
{
	wxString raw;
	wxString msg;
	bool chkHead = false;
	bool chkSum = false;
	int scale;
	int index = 0;
	int expHead;
	int actHead;

	// convert pid to wxString
	scale = ((pid / 255) + 1) * 2;		// number of 'bytes' * 2
	wxString pidString = wxString::Format(_T("%0*x"), scale, pid);

	// send to device
	if (this->obdWrite(pidString, pidString.length())) {

//		sleep(10);
//		usleep(20000);
		// read return from device
		raw = this->obdRead();

		// write the raw response to the log if needed
		if (logger && logExtra) {
			msg.Printf(_("Raw data: %s\n"), raw.c_str());
			logger->appendLog(msg, logPanel::LOG_IN);
		}

		// tokenise the string
		wxStringTokenizer tkz(raw, wxT(" "));
		while ( tkz.HasMoreTokens()  && index <= toksize )
		{
			wxString token = tkz.GetNextToken();
			token.ToLong((long int*)&tokens[index], 16);
			index++;
		}

		// check header
		scale = (scale / 2);			// redefine scale as no of 'bytes' expected
		expHead = pid + (64 * pow(256, (scale - 1)));
		actHead = 0;
		for (int i = 0; i < scale; i++) {
			actHead += pow(256, (scale - 1 - i)) * tokens[i];
		}
		if (actHead == expHead) {
			chkHead = true;
		}

		// check checksum
		if (useChecksum) {
			// TODO: Calculate and check the checksum
			chkSum = true;
		} else {
			chkSum = true;
		}
	}

	return (chkHead && chkSum);
}

/// \brief Get the value associated with a PID from the ECU
///
/// \param[in] pid The pid you wish to enquire
/// \param[out] result Pointer to receive the calculated value
/// \result True if a value has been successfully retreived
bool obdbase::obd_pid_value(int pid, obdbase::pidInfo* result)
{
    bool retVal = false;
    int toks[40];
    wxString msg;

    // set sensible defaults
    result->pid_flag = PID_FLAG_SINGLE;
    result->resultMain = 0;
    result->resultSecondary = 0;
    result->resultString.Empty();

	// write to log if necessary
	if (logger) {
		msg.Printf(_("Requesting PID: %#.4x\n"), pid);
		logger->appendLog(msg, logPanel::LOG_OUT);
	}

    // ask for the raw data
    if (this->obd_pid_get_raw(pid, toks, sizeof(toks))) {
        // we have a good result so...
        retVal = true;

        switch (pid) {
            case PID_MAP:
            case PID_VSS:
            case PID_WARM_UPS:
            case PID_BARO:
                result->resultMain = toks[2];
                break;
            case PID_BANK1_STFT:
            case PID_BANK1_LTFT:
            case PID_BANK2_STFT:
            case PID_BANK2_LTFT:
            case PID_EGR_ERR:
                result->resultMain = (toks[2] - 128) * 100 / 128;
                break;
            case PID_ECT:
            case PID_IAT:
            case PID_AAT:
                result->resultMain = toks[2] - 40;
                break;
            case PID_MIL_DIST:
            case PID_CLR_DIST:
            case PID_MIL_TIME:
            case PID_CLR_TIME:
            case PID_RUNTM:
                result->resultMain = (toks[2] * 256) + toks[3];
                break;
            case PID_FRP:
                result->resultMain = toks[2] * 3;
                break;
            case PID_SPARKADV:
                result->resultMain = (toks[2] / 2) - 64;
                break;
            case PID_MAF:
                result->resultMain = ((toks[2] * 256) + toks[3]) / 100;
                break;
            case PID_TP:
            case PID_TP_R:
            case PID_TP_B:
            case PID_TP_C:
            case PID_APP_D:
            case PID_APP_E:
            case PID_APP_F:
            case PID_LOAD_PCT:
            case PID_EGR_PCT:
            case PID_EVAP_PCT:
            case PID_FLI:
            case PID_TAC_PCT:
            case PID_ALCH_PCT:
            case PID_APP_R:
                result->resultMain = toks[2] * 100 / 255;
                break;
            case PID_RPM:
            case PID_EVAP_VP:
                result->resultMain = ((toks[2] * 256) + toks[3]) / 4;
                break;
            case PID_O2S11:
            case PID_O2S12:
            case PID_O2S13:
            case PID_O2S14:
            case PID_O2S21:
            case PID_O2S22:
            case PID_O2S23:
            case PID_O2S24:
                result->resultMain = toks[2] * 0.005;
                result->resultSecondary = (toks[3] - 128) * 100 / 128;
                result->pid_flag = PID_FLAG_DOUBLE;
                break;
            case PID_CATEMP11:
            case PID_CATEMP21:
            case PID_CATEMP12:
            case PID_CATEMP22:
                result->resultMain = ((toks[2] * 256) + toks[3]) / 10 - 40;
                break;
            case PID_FRP_REL:
                result->resultMain = (((toks[2] * 256) + toks[3]) * 10) / 128;
                break;
            case PID_FRP_ATMO:
            case PID_FRP_ABS:
                result->resultMain = ((toks[2] * 256) + toks[3]) * 10;
            case PID_AIR_STAT:
                result->resultString = obd_pid_air_stat(toks[2]);
                result->pid_flag = PID_FLAG_STRING;
                break;
            case PID_O2SLOC:
                //TODO: Support PID_O2SLOC
                result->resultString = _("Not yet supported");
                result->pid_flag = PID_FLAG_STRING;
                break;
            case PID_OBDSUP:
                result->resultString = this->obd_pid_obd_supported(toks[2]);
                result->pid_flag = PID_FLAG_STRING;
                break;
            case PID_PTO_STAT:
                if (toks[2] == 0x80) {
                    result->resultString = _T("ON");
                } else {
                    result->resultString = _T("OFF");
                }
                result->pid_flag = PID_FLAG_STRING;
                break;
            case PID_VPWR:
                result->resultMain = ((toks[2] * 256) + toks[3]) / 1000;
                break;
            case PID_LOAD_ABS:
                result->resultMain = ((toks[2] * 256) + toks[3]) * 100 / 255;
                break;
            case PID_EQ_RAT:
                result->resultMain = ((toks[2] * 256) + toks[3]) / 32768;
                break;
            case PID_FUEL_TYP:
                result->resultString = obd_pid_fuel_type(toks[2]);
                result->pid_flag = PID_FLAG_STRING;
                break;
            case PID_VIN:
                result->resultString = obd_pid_vin(toks);
                result->pid_flag = PID_FLAG_STRING;
                break;
            case PID_EVAP_VPA:
                result->resultMain = ((toks[2] * 256) + toks[3]) / 200;
                break;
            default:
                // bail out if we don't support the PID in this function
                retVal = false;
        }

        // write to log if necessary
        if (logger) {
            msg.Printf(_("Result for PID(%#.4x): %f\n"), pid, result->resultMain);
            logger->appendLog(msg, logPanel::LOG_IN);
        }
    } else {
		// write to log if necessary
		if (logger) {
			msg.Printf(_("Could not get result for PID: %#.4x\n"), pid);
			logger->appendLog(msg, logPanel::LOG_ERROR);
		}
	}

    if (this->useImperial) {
        this->convertToImperial(pid, result);
    }

    return retVal;
}   // obd_pid_value()

/// \brief Get the PIDS/OBDMIDS supported by the ECU
///
/// Not all ECUs support the whole range of Mode 0x01 PIDs.  This
/// function will parse the 3 PID which provide the full list of PIDS
/// supported by this particular ECU.
///
/// \param[in] mode The modes we wish the list of supports for
/// \param[out] pids A vector to receive the supported PIDs.
/// \since 0.3.3
void obdbase::obdSupportedPids(int mode, std::vector<int>& pids)
{
    std::vector<int> retVal;
    int start = 0x100 * mode;
    bool stopCondition = false;

    while (stopCondition == false) {

        int toks[7];
        unsigned int bitMask = 0x80000000;
        unsigned int indicator;

        if (this->obd_pid_get_raw(start, toks, sizeof(toks))) {

            unsigned int pidEncoded = (toks[2] * pow(256, 3)) + (toks[3] * pow(256, 2)) + (toks[4] * 256) + toks[5];

            for (int i = start; i <= start + 0x20; i++) {
                // apply the bit mask
                indicator = pidEncoded & bitMask;
                // result will be == 0, or > 0
                if (indicator > 0) {
                    retVal.push_back(start + i);
                }
                // get the bitmask ready for the next loop
                bitMask = bitMask / 2;
            }
        }

        // do we have a stop condition
        start += 0x20;
        if (!retVal.empty() && retVal.back() == start) {
            retVal.pop_back();
        } else {
            stopCondition = true;
        }
    }

    // copy the retrieved list into the class variable
    pids.swap(retVal);

}   // obd_pid_supported_pids()

/// \brief Determine if the serial port is open
///
///
///
/// \return True if the serial port is open
bool obdbase::obd_is_connected ()
{
    bool retval;

    if (port->IsOpen() == 1) {
        retval = true;
    } else {
        retval = false;
    }

    return retval;
}

/// \brief Convert the results to imperial
///
/// \param[in] pid The pid we are currently working with
/// \param[in,out] result The result in metric, returned in imperial
/// \since 0.5.1
void obdbase::convertToImperial(int pid, obdbase::pidInfo* result)
{
    switch (pid) {
        case PID_ECT:
        case PID_IAT:
        case PID_CATEMP11:
        case PID_CATEMP12:
        case PID_CATEMP21:
        case PID_CATEMP22:
        case PID_AAT:
            result->resultMain = this->convert_c_to_f(result->resultMain);
            break;
        case PID_VSS:
        case PID_CLR_DIST:
        case PID_MIL_DIST:
            result->resultMain = this->convert_kph_to_mph(result->resultMain);
            break;
        case PID_FRP:
        case PID_FRP_ATMO:
        case PID_FRP_REL:
        case PID_FRP_ABS:
            result->resultMain =  this->convert_kpa_to_psi(result->resultMain);
            break;
        case PID_EVAP_VP:
            result->resultMain = this->convert_pa_to_inh20(result->resultMain);
            break;
        case PID_MAP:
            result->resultMain = this->convert_pa_to_inhg(result->resultMain);
            break;
        case PID_BARO:
            result->resultMain = this->convert_kpa_to_inhg(result->resultMain);
            break;
        case PID_MAF:
            result->resultMain = this->convert_gs_to_lbmin(result->resultMain);
            break;
        case PID_EVAP_VPA:
            result->resultMain = this->convert_kpa_to_inh20(result->resultMain);
            break;
    }
}

/// \brief Convert temperature from centigrade to farenheit
///
/// \param[in] centigrade Temperature in degrees centigrade.
/// \return Temperature in degrees farenheit.
int obdbase::convert_c_to_f (int centigrade)
{
	return (9 / 5) * centigrade + 32;
}

/// \brief Convert pressure from kPa to PSI.
///
/// \param[in] kpa Pressure in kPa
/// \return Pressure in PSI
/// \since 0.5.1
double obdbase::convert_kpa_to_psi(double kpa)
{
    return kpa * 0.1450377;
}

/// \brief Convert kilometres to miles
///
/// \param[in] kilometres Distance or speed in kilometres.
/// \return distance or speed in miles.
int obdbase::convert_kph_to_mph (int kph)
{
	return kph / 1.60934400061;
}

/// \brief Convert Pa to in H20
///
/// \param[in] pa Pressure in Pa
/// \return Pressure in inches of H20
/// \since 0.5.1
double obdbase::convert_pa_to_inh20(double pa)
{
    return pa * 0.0040146309;
}

/// \brief Convert g/s to lb/s
///
/// \param[in] gs Mass air flow rate in grammes per second
/// \return Mass air flow rate in pounds per second
/// \since 0.5.1
double obdbase::convert_gs_to_lbs(double gs)
{
    return gs * 0.0022046;
}

/// \brief Convert Pa to in Hg
///
/// \param pa Pressure in Pascals
/// \return Pressure in inches of Mercury
/// \since 0.5.1
double obdbase::convert_pa_to_inhg(double pa)
{
    // TODO: Get conversion formula pa->inHG
    return pa;
}

/// \brief Convert grammes per second to pounds per minute
///
/// \param[in] gs Air flow rate in grammes per second
/// \return Air flow rate in pounds per minute
/// \since 0.5.1
double obdbase::convert_gs_to_lbmin(double gs)
{
    return this->convert_gs_to_lbs(gs) * 60;
}

/// \brief Convert kPa to inHg
///
/// \param[in] kpa Pressure in kPa
/// \return Pressure in inHg
/// \since 0.5.1
double obdbase::convert_kpa_to_inhg(double kpa)
{
    return this->convert_pa_to_inhg(kpa * 1000);
}

/// \brief Convert kPa to in H20
///
/// \param kpa Pressure in kPa
/// \return Pressure in in H20
/// \since 0.5.1
double obdbase::convert_kpa_to_inh20(double kpa)
{
    return this->convert_pa_to_inh20(kpa * 1000);
}

/// \brief Decodes the Commanded Secondary Air Status.
///
/// \param[in] encByte A byte encoded with OBD Standards Commanded
/// Air Status
/// \return A string representation of the AIR_STAT
/// \since 0.5.1
wxString obdbase::obd_pid_air_stat(int encByte)
{
    wxString retVal;

    switch (encByte) {
        case 0x80:
            retVal = _T("UPS");
            break;
        case 0x040:
            retVal = _T("DNS");
            break;
        case 0x20:
            retVal = _T("OFF");
            break;
        default:
            retVal = _("Illegal value");
    }

    return retVal;
}

/// \brief Decodes the OBD protocols supported by a vehicle
///
/// \param[in] encByte A byte encoded to OBD-II standards representing
/// the protocols supported.
/// \return A string representation of OBDSUP.
/// \since 0.5.1
wxString obdbase::obd_pid_obd_supported(int encByte)
{
    wxString retVal;

    switch (encByte) {
        case 0x01:
            retVal = _T("OBD II");
            break;
        case 0x02:
            retVal = _T("OBD");
            break;
        case 0x03:
            retVal = _T("OBD and OBD II");
            break;
        case 0x04:
            retVal = _T("OBD I");
            break;
        case 0x05:
            retVal = _T("NO OBD");
            break;
        case 0x06:
            retVal = _T("EOBD");
            break;
        case 0x07:
            retVal = _T("EOBD and OBD II");
            break;
        case 0x08:
            retVal = _T("EOBD and OBD");
            break;
        case 0x09:
            retVal = _T("EOBD, OBD and OBD II");
            break;
        case 0x0A:
            retVal = _T("JOBD");
            break;
        case 0x0B:
            retVal = _T("JOBD and OBD II");
            break;
        case 0x0C:
            retVal = _T("JOBD and EOBD");
            break;
        case 0x0D:
            retVal = _T("JOBD, EOBD, and OBD II");
            break;
        case 0x0E:
            retVal = _T("EURO IV B1");
            break;
        case 0x0F:
            retVal = _T("EURO V B2");
            break;
        case 0x10:
            retVal = _T("EURO C");
            break;
        case 0x11:
            retVal = _T("EMD");
            break;
        default:
            retVal = _("Illegal value");
    }

    return retVal;
}

/// \brief Decodes the fuel type in use by the vehicle.
///
/// \param[in] encByte A byte encoded to OBD-II standards describing the
/// fuel type in use.
/// \return A string representation of the fuel type currently in use.
/// \since 0.5.1
wxString obdbase::obd_pid_fuel_type(int encByte)
{
    wxString retVal;

    switch (encByte) {
        case 0x01:
            retVal = _T("GAS");
            break;
        case 0x02:
            retVal = _T("METH");
            break;
        case 0x03:
            retVal = _T("ETH");
            break;
        case 0x04:
            retVal = _T("DSL");
            break;
        case 0x05:
            retVal = _T("LPG");
            break;
        case 0x06:
            retVal = _T("CNG");
            break;
        case 0x07:
            retVal = _T("PROP");
            break;
        case 0x08:
            retVal = _T("ELEC");
            break;
        case 0x09:
            retVal = _T("BI_GAS");
            break;
        case 0x0A:
            retVal = _T("BI_METH");
            break;
        case 0x0B:
            retVal = _T("BI_ETH");
            break;
        case 0x0C:
            retVal = _T("BI_LPG");
            break;
        case 0x0D:
            retVal = _T("BI_CNG");
            break;
        case 0x0E:
            retVal = _T("BI_PROP");
            break;
        case 0x0F:
            retVal = _T("BI_ELEC");
            break;
        default:
            retVal = _("Illegal value");
    }

    return retVal;
}

/// \brief Decodes PID_VIN
///
/// \param[in] tokens An array of bytes returned by the ECU.
/// \return A string representation of the VIN.
/// \since 0.5.1
/// \todo The CAN standard is different and currently not supported.
wxString obdbase::obd_pid_vin(int tokens[])
{
    wxString retVal;

    for (int i = 6; i < 35; i++) {
        retVal.Append(tokens[i]);
    }

    return retVal;
}
