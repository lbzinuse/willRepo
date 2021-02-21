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

/// \class elm327
/// \brief A class for interacting with an ELM327 OBD-II interface device

#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "elm327.h"
#include "obdbase.h"
#include "logPanel.h"
#include "ctb-0.15/ctb.h"

using namespace std;
using namespace ctb;

elm327::elm327 (const wxString& serialPort) : obdbase(serialPort)
{
	this->obdInitSlow();
	this->elmSetEcho(false);
	this->elmSetHeaders(false);
	this->version_ = 0.0;
}

/// \brief Request a change of protocol
///
/// Ask the ELM device to change the protocol used to communicate with
/// the ECU.
///
/// \param OBDprotocol The protocol to change to
/// \return True if the device reports a change.
bool elm327::obdProtocolSet (int OBDprotocol)
{
	bool result = false;

	if (logger) {
		wxString msg;
		msg.Printf(_("Asking to change protocol to %02X\n"), OBDprotocol);
		logger->appendLog(msg, logPanel::LOG_OUT);
	}

	wxString cmd = wxString::Format(_T("SP %x"), OBDprotocol);
	wxString response = this->elmSendAtCommand(cmd);

	if (response.CmpNoCase(_T("OK"))) {
		result = true;

		// preform an init
		this->obdInitSlow();
	}
	return result;
}

/// \brief Ask the ELM device for its identity
///
///
///
/// \return A string containing the identity reported by the device
wxString elm327::obdDeviceIdentify()
{
    wxString result;
    double version;

	if (logger) {
		wxString msg = _("Asking device for identity\n");
		logger->appendLog(msg, logPanel::LOG_OUT);
	}

    // ask the device for identify
	wxString cmd(_T("I"));
	result = this->elmSendAtCommand(cmd);

	// update the version number
	result.AfterLast('v').ToDouble(&version);
	this->version_ = version;

	// return the identify
	return result;
}

wxString elm327::obdProtocolGet()
{
	if (logger) {
		wxString msg = _("Asking device for current protocol\n");
		logger->appendLog(msg, logPanel::LOG_OUT);
	}

	wxString cmd(_T("DP"));
	return this->elmSendAtCommand (cmd);
}

void elm327::obdInitSlow ()
{
	if (logger) {
		wxString msg = _("Asking device for initialisation (slow)\n");
		logger->appendLog(msg, logPanel::LOG_OUT);
	}

	wxString cmd(_T("SI"));
	this->elmSendAtCommand (cmd);
}

void elm327::obdInitFast ()
{
	if (logger) {
		wxString msg = _("Asking device for initialisation (fast)\n");
		logger->appendLog(msg, logPanel::LOG_OUT);
	}

	wxString cmd(_T("FI"));
	this->elmSendAtCommand(cmd);
}

/// \brief Disconnect from the serial port
///
/// As per base class, but also needs to reset ELM specific parameters
///
/// \see obdbase::obd_device_disconnect()
/// \since 0.5.1
void elm327::obdDeviceDisconnect()
{
    obdbase::obdDeviceDisconnect();
    version_ = 0.0;
}

wxString elm327::elmSendAtCommand (const wxString& command)
{
	wxString result;
	wxString fullCmd;

	// Build the full command
	fullCmd = _T("AT") + command;

	if (this->obdWrite(fullCmd, fullCmd.length())) {
		result = this->obdRead();
	} else {
		result = _("Communication error");
	}

	return result;
}

/// \brief Turn on or off the header responses
///
/// \param show If we wish to see the headers or not
/// \return True is the device confirms the change
/// \since 0.3.0
bool elm327::elmSetHeaders (bool show)
{
	wxString msg;
    wxString result;
    bool retVal = false;

    // ask the device to show or not show headers
	if (show) {
		msg = _T("H1");
	} else {
		msg = _T("H0");
	}
	result = elmSendAtCommand(msg);

	// check we have a good result
	if (result.Cmp(_T("OK")) == 0) {
	    retVal = true;
	}

	// return our success
	return retVal;
}

/// \brief Turn echoing of request on or off
///
/// \param show If we wish to show the echos or not
/// \return True if the device responds affirmatively to the change
/// \since 0.3.0
bool elm327::elmSetEcho(bool show)
{
	wxString msg;
	wxString result;
	bool retVal = false;

	// ask the device to set the echo and collect the result
	if (show) {
		msg = _T("E1");
	} else {
		msg = _T("E0");
	}
	result = elmSendAtCommand(msg);

	// check we have a good result
	if (result.Cmp(_T("OK")) == 0) {
	    retVal = true;
	}

	// return our success
	return retVal;
}

/// \brief Switch can auto-formatting on or of
///
/// \param[in] on True to switch auto-formatting on
/// \since 0.5.1
bool elm327::elmSetCanAutoformat(bool on)
{
    wxString msg;
    wxString result;
    bool retVal = false;

    // send the AT command and pickup the response
	if (on) {
		msg = _T("CAF1");
	} else {
		msg = _T("CAF0");
	}
	result = elmSendAtCommand(msg);

    // if the reponse is OK then success
	if (result.Cmp(_T("OK")) == 0) {
	    retVal = true;
	}

    // return whether successful
	return retVal;
}

/// \brief Get the version of the ELM device
///
/// There have been several versions of the ELM327, with each one
/// having progressively more functionality
/// \return The version of the ELM device
/// \since 0.5.1
double elm327::elmGetVersion()
{
    // if we haven't already got a version, ask the device
    // to identify itself
    if (this->obd_is_connected() && version_ == 0.0) {
        this->obdDeviceIdentify();
    }

    // return teh version
    return this->version_;
}
