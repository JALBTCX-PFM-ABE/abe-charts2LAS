
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/


#ifndef VERSION

#define     VERSION     "PFM Software - charts2LAS V2.06 - 08/26/16"

#endif


/*! <pre>

    Version 1.00
    Jan C. Depner
    07/18/11

    First version.


    Version 1.01
    Jan C. Depner
    07/27/11

    Added option to output invalid data as well as valid data.  Also, added _h or _t to the output file name to 
    differentiate between hydro and topo data.


    Version 1.02
    Jan C. Depner
    08/29/11

    Added land, water, invalid classifications for HOF data.


    Version 1.03
    Jan C. Depner
    10/06/11

    Added land, water, invalid classifications for HOF data.


    Version 1.04
    Jan C. Depner
    10/06/11

    To match a change in the TOF PFM loader we now check input files start times against 10/07/2011.
    Files prior to that date will ignore the first return if the last return was -998.0.


    Version 1.05
    Jan C. Depner
    10/21/11

    Changed ACE classification values to 21, 27, and 29 from 101, 107, and 109.


    Version 1.06
    Jan C. Depner
    11/30/11

    Converted .xpm icons to .png icons.
    

    Version 1.07
    Chris Macon
    01/18/12
    
    Added statement to correct the return number when the 2nd return was valid but the 1st return was invalid
    (Making it a "1 of 1" instead of "2 of 1")


    Version 1.08
    Jan C. Depner (PFM Software)
    12/09/13

    Switched to using .ini file in $HOME (Linux) or $USERPROFILE (Windows) in the ABE.config directory.  Now
    the applications qsettings will not end up in unknown places like ~/.config/navo.navy.mil/blah_blah_blah on
    Linux or, in the registry (shudder) on Windows.


    Version 1.09
    Jan C. Depner (PFM Software)
    01/06/14

    Fixed an obvious screwup in inputPage.cpp where I was re-using the loop counters.
    Scoping in C++ saved my bacon but it was just too damn confusing.


    Version 1.10
    Jan C. Depner (PFM Software)
    01/07/14

    Switched from geoid03 to geoid12a.


    Version 1.11
    Jan C. Depner (PFM Software)
    01/11/14

    Fixed the year in the LAS header.


    Version 1.12
    Jan C. Depner (PFM Software)
    07/01/14

    - Replaced all of the old, borrowed icons with new, public domain icons.  Mostly from the Tango set
      but a few from flavour-extended and 32pxmania.


    Version 1.20
    Jan C. Depner (PFM Software)
    07/11/14

    - Added the option of using GPS time instead of GPS seconds of the week.
    - After years of trying to get liblas 1.2.1 to work properly with VLRs I've finally given up.
      I now write the VLR and VLR counr directly to the file myself.


    Version 1.21
    Jan C. Depner (PFM Software)
    07/17/14

    - No longer uses liblas.  Now uses libslas (my own API for LAS).


    Version 1.22
    Jan C. Depner (PFM Software)
    07/23/14

    - Switched from using the old NV_INT64 and NV_U_INT32 type definitions to the C99 standard stdint.h and
      inttypes.h sized data types (e.g. int64_t and uint32_t).


    Version 1.23
    Jan C. Depner (PFM Software)
    12/02/14

    Added option to select GEOID03, GEOID09, or GEOID12a for orthometric correction.


    Version 1.24
    Jan C. Depner (PFM Software)
    01/06/15

    Added code to deal with new leap second to be added on June 30th, 2015 at 23:59:60.


    Version 2.00
    Jan C. Depner (PFM Software)
    03/12/15

    Switched to using rapidlasso GmbH LASlib and LASzip instead of my libslas.


    Version 2.01
    Jan C. Depner (PFM Software)
    03/17/15

    - Setting the vertical datum properly.
    - Removed option for setting time to seconds of the week.


    Version 2.02
    Jan C. Depner (PFM Software)
    07/03/15

    - Finally straightened out the GPS time/leap second problem (I hope).


    Version 2.03
    Jan C. Depner (PFM Software)
    07/07/15

    - Fixed creation year and number of points by return.


    Version 2.04
    Jan C. Depner (PFM Software)
    04/05/16

    - Added geoid12b option.


    Version 2.05
    Jan C. Depner (PFM Software)
    04/29/16

    - Now that we have the ability to modify the classification field in ABE (and unload it in pfm_unload) this program
      will now translate the classification_status field to LAS 1.2 if it is anything other than 0.


    Version 2.06
    Jan C. Depner (PFM Software)
    08/26/16

    - Now uses the same font as all other ABE GUI apps.  Font can only be changed in pfmView Preferences.

</pre>*/
