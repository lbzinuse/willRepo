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

#ifndef __logPanel__
#define __logPanel__

/**
@file
Subclass of logBasePanel, which is generated by wxFormBuilder.
*/

#include "gui.h"

/** Implementing logBasePanel */
class logPanel : public logBasePanel
{
public:
    enum logType {LOG_IN, LOG_OUT, LOG_ERROR, LOG_OTHER};

	/** Constructor */
	logPanel( wxWindow* parent );

	void appendLog(wxString& logText, logType type);
	void SaveFile(wxString& path);
};

#endif // __logPanel__
