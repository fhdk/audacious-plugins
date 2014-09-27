/*
 * Audacious: A cross-platform multimedia player
 * Copyright (c) 2006-2010 William Pitcock, Tony Vroon, George Averill, Giacomo
 *  Lozito, Derek Pomery and Yoshiki Yazawa, and John Lindgren.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <string.h>

#include <libaudcore/audstrings.h>
#include <libaudcore/i18n.h>
#include <libaudcore/plugin.h>

static const char * const m3u_exts[] = {"m3u", "m3u8"};

class M3ULoader : public PlaylistPlugin
{
public:
    static constexpr PluginInfo info = {N_("M3U Playlists"), PACKAGE};

    constexpr M3ULoader () : PlaylistPlugin (info, m3u_exts, true) {}

    bool load (const char * filename, VFSFile & file, String & title,
     Index<PlaylistAddItem> & items);
    bool save (const char * filename, VFSFile & file, const char * title,
     const Index<PlaylistAddItem> & items);
};

M3ULoader aud_plugin_instance;

static void strip_char (char * text, char c)
{
    char * set = text;
    char a;

    while ((a = * text ++))
        if (a != c)
            * set ++ = a;

    * set = 0;
}

static Index<char> read_win_text (VFSFile & file)
{
    Index<char> raw = file.read_all ();
    if (! raw.len ())
        return raw;

    raw.append (0);  /* null-terminated */
    strip_char (raw.begin (), '\r');
    return raw;
}

static char * split_line (char * line)
{
    char * feed = strchr (line, '\n');
    if (! feed)
        return nullptr;

    * feed = 0;
    return feed + 1;
}

bool M3ULoader::load (const char * path, VFSFile & file, String & title,
 Index<PlaylistAddItem> & items)
{
    Index<char> text = read_win_text (file);
    if (! text.len ())
        return false;

    char * parse = text.begin ();

    while (parse)
    {
        char * next = split_line (parse);

        while (* parse == ' ' || * parse == '\t')
            parse ++;

        if (* parse && * parse != '#')
        {
            StringBuf s = uri_construct (parse, path);
            if (s)
                items.append (String (s));
        }

        parse = next;
    }

    return true;
}

bool M3ULoader::save (const char * path, VFSFile & file, const char * title,
 const Index<PlaylistAddItem> & items)
{
    for (auto & item : items)
    {
        StringBuf line = str_concat ({item.filename, "\n"});
        if (file.fwrite (line, 1, line.len ()) != line.len ())
            return false;
    }

    return true;
}