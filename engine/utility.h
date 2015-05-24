//
// C++ Interface: utility
//
// Description: 
//
//	GiadaEngine is a derivative of Giada
//
// Copyright (C) 2015 John D. Derry
//
//
/* ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ------------------------------------------------------------------ */


#ifndef __UTILITY_H__
#define __UTILITY_H__

/* -- log modes ----------------------------------------------------- */
#define LOG_MODE_STDOUT 0x01
#define LOG_MODE_FILE   0x02
#define LOG_MODE_MUTE   0x04


/* gLog  
 *
 * init logger. Mode defines where to write the output: LOG_MODE_STDOUT,
 * LOG_MODE_FILE and LOG_MODE_MUTE. */
 
int  gLog_init (int mode);

void gLog_close();

void gLog(const char *format, ...);


/**********************************************************************/


/* gVector
 * lightweight template class. */

template <class T> class gVector {
public:


	/* gVector()
	 * default constructor, no parameters */

	gVector() : size(0), s(NULL) {}

	/* gVector(const &)
	 * copy-constructor, when gVector a = b (where b is gVector).
	 * Default constructor doesn't copy referenced ojbects, so we need
	 * to re-allocate the internal stack for the copied object */

	gVector(const gVector &other) {
		s = new T[other.size];
		for (unsigned i=0; i<other.size; i++)
			s[i] = other.s[i];
		size = other.size;
	}


	~gVector() {
		/// FIXME empty s with clear()?!?
	}


	void add(const T &item) {
		T *tmp = new T[size+1];  /// TODO: chunk increment (size+N), N ~= 16
		for (unsigned i=0; i<size; i++)
			tmp[i] = s[i];
		tmp[size] = item;
		delete[] s;
		s = tmp;
		size++;
	}


	int del(const T &item) {
		for (unsigned i=0; i<size; i++)
			if (s[i] == item)
				return del(i);
		return -1;
	}


	int del(unsigned p) {
		if (p > size-1) gLog("[vector] del() outside! requested=%d, size=%d\n", p, size);
		T *tmp = new T[size-1];
		unsigned i=0;
		unsigned j=0;
		while (i<size) {
			if (i != p) {
				tmp[j] = s[i];
				j++;
			}
			i++;
		}
		delete[] s;
		s = tmp;
		size -= 1;
		return size;
	}


	void clear() {
		if (size > 0) {
			delete [] s;
			s = NULL;
			size = 0;
		}
	}


	void swap(unsigned x, unsigned y) {
		T tmp = s[x];
		s[x] = s[y];
		s[y] = tmp;
	}


	T &at(unsigned p) {
		if (p > size-1)	gLog("[vector] at() outside! requested=%d, size=%d\n", p, size);
		return s[p];
	}


	T &last() {
		return s[size-1];
	}


	unsigned size;
	T *s;  				// stack (array of T)
};


/* ---------------------------------------------------------------------------- */

/*
 *  BUNCH OF SIMILIAR STUFF
 */

bool gFileExists(const char *path);

bool gDirExists(const char *path);

bool gIsDir(const char *path);

bool gIsProject(const char *path);

bool gIsPatch(const char *path);

bool gMkdir(const char *path);

std::string gBasename(const char *path);

std::string gReplace(std::string in, const std::string& search, const std::string& replace);

std::string gDirname(const char *path);

std::string gTrim(const char *path);
std::string gTrim(const std::string &s);

std::string gGetCurrentPath();

std::string gGetHomePath();

std::string gStripFileUrl(const char *path);

std::string gGetExt(const char *path);

std::string gStripExt(const char *path);

std::string gGetProjectName(const char *path);

std::string gGetSlash();

std::string gItoa(int i);

/* ---------------------------------------------------------------------------- */

#endif
