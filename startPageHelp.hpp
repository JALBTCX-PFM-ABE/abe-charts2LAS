
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



QString invText = 
  startPage::tr ("If this box is checked invalid data will be output along with valid data.  If an invalid point is output "
                 "to the LAS file its <b>User Data</b> field will be set to <b>I</b> for invalid.");

QString ortho03Text = 
  startPage::tr ("If this button is toggled on then we will attempt to correct the ellipsoidal height to an orthometric height "
                 "using the National Geodetic Survey (NGS) GEOID03 files.  The files must be stored in a directory called geoid_data that "
                 "is in the directory pointed to by the environment variable ABE_DATA.  The "
                 "files must be the binary versions (not ASCII) and the file names must be the following:<br><br>"
                 "<ul>"
                 "<li>g2003a01.bin</li>"
                 "<li>g2003a02.bin</li>"
                 "<li>g2003a03.bin</li>"
                 "<li>g2003a04.bin</li>"
                 "<li>g2003u01.bin</li>"
                 "<li>g2003u02.bin</li>"
                 "<li>g2003u03.bin</li>"
                 "<li>g2003u04.bin</li>"
                 "<li>g2003u05.bin</li>"
                 "<li>g2003u06.bin</li>"
                 "<li>g2003u07.bin</li>"
                 "<li>g2003u08.bin</li>"
                 "<li>g2003h01.bin</li>"
                 "<li>g2003p01.bin</li>"
                 "</ul><br><br>"
                 "<b>IMPORTANT NOTE: This option implies that the output file will be relative to GCS_NAD83 and only works with "
                 "data collected in KGPS mode.</b>");

QString ortho09Text = 
  startPage::tr ("If this button is toggled on then we will attempt to correct the ellipsoidal height to an orthometric height "
                 "using the National Geodetic Survey (NGS) GEOID09 files.  The files must be stored in a directory called geoid_data that "
                 "is in the directory pointed to by the environment variable ABE_DATA.  The "
                 "files must be the binary versions (not ASCII) and the file names must be the following:<br><br>"
                 "<ul>"
                 "<li>g2009a01.bin</li>"
                 "<li>g2009a02.bin</li>"
                 "<li>g2009a03.bin</li>"
                 "<li>g2009a04.bin</li>"
                 "<li>g2009u01.bin</li>"
                 "<li>g2009u02.bin</li>"
                 "<li>g2009u03.bin</li>"
                 "<li>g2009u04.bin</li>"
                 "<li>g2009u05.bin</li>"
                 "<li>g2009u06.bin</li>"
                 "<li>g2009u07.bin</li>"
                 "<li>g2009u08.bin</li>"
                 "<li>g2009h01.bin</li>"
                 "<li>g2009p01.bin</li>"
                 "<li>g2009g01.bin</li>"
                 "<li>g2009s01.bin</li>"
                 "</ul><br><br>"
                 "<b>IMPORTANT NOTE: This option implies that the output file will be relative to GCS_NAD83 and only works with "
                 "data collected in KGPS mode.</b>");

QString ortho12aText = 
  startPage::tr ("If this button is toggled on then we will attempt to correct the ellipsoidal height to an orthometric height "
                 "using the National Geodetic Survey (NGS) GEOID12a files.  The files must be stored in a directory called geoid_data that "
                 "is in the directory pointed to by the environment variable ABE_DATA.  The "
                 "files must be the binary versions (not ASCII) and the file names must be the following:<br><br>"
                 "<ul>"
                 "<li>g2012au0.bin</li>"
                 "<li>g2012aa0.bin</li>"
                 "<li>g2012ah0.bin</li>"
                 "<li>g2012ap0.bin</li>"
                 "<li>g2012ag0.bin</li>"
                 "<li>g2012as0.bin</li>"
                 "</ul><br><br>"
                 "<b>IMPORTANT NOTE: This option implies that the output file will be relative to GCS_NAD83 and only works with "
                 "data collected in KGPS mode.</b>");

QString ortho12bText = 
  startPage::tr ("If this button is toggled on then we will attempt to correct the ellipsoidal height to an orthometric height "
                 "using the National Geodetic Survey (NGS) GEOID12b files.  The files must be stored in a directory called geoid_data that "
                 "is in the directory pointed to by the environment variable ABE_DATA.  The "
                 "files must be the binary versions (not ASCII) and the file names must be the following:<br><br>"
                 "<ul>"
                 "<li>g2012bu0.bin</li>"
                 "<li>g2012ba0.bin</li>"
                 "<li>g2012bh0.bin</li>"
                 "<li>g2012bp0.bin</li>"
                 "<li>g2012bg0.bin</li>"
                 "<li>g2012bs0.bin</li>"
                 "</ul><br><br>"
                 "<b>IMPORTANT NOTE: This option implies that the output file will be relative to GCS_NAD83 and only works with "
                 "data collected in KGPS mode.</b>");

QString compText = 
  startPage::tr ("If this box is checked, the output files will be stored using <b>rapidlasso GmbH</b> LASzip format.  They will have a file "
                 "extension of .laz instead of the normal .las extension.");
