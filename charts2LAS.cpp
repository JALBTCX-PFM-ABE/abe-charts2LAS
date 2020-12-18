
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



#include "charts2LAS.hpp"

double settings_version = 2.0;


charts2LAS::charts2LAS (QWidget *parent)
  : QWizard (parent)
{
  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/charts2LASWatermark.png"));


  envin ();


  // Set the application font

  QApplication::setFont (font);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, true);
  setOption (ExtendedWatermarkPixmap, false);


  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (window_width, window_height);
  this->move (window_x, window_y);

  setPage (0, new startPage (this, invalid, &ortho, compress));
  setPage (1, new fileInputPage (this, &inputFileFilter, &inputFiles, input_dir));
  setPage (2, new runPage (this, &progress));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, true);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start the conversion process"));
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}



charts2LAS::~charts2LAS ()
{
}



void charts2LAS::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.png"));
  button (QWizard::CustomButton1)->setEnabled (false);

  switch (id)
    {
    case 0:
      input_files.clear ();
      break;

    case 1:
      break;

    case 2:

      button (QWizard::CustomButton1)->setEnabled (true);

      invalid = field ("invalid").toBool ();
      compress = field ("compress").toBool ();


      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      window_x = tmp.x ();
      window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      window_width = tmp.width ();
      window_height = tmp.height ();


      envout ();


      break;
    }
}



void charts2LAS::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;
    }
}



//  This is where the fun stuff happens.

void 
charts2LAS::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (false);
  button (QWizard::BackButton)->setEnabled (false);
  button (QWizard::CustomButton1)->setEnabled (false);


  //  Get the files from the QTextEdit box on the fileInputPage.

  QTextCursor inputCursor = inputFiles->textCursor ();

  inputCursor.setPosition (0);


  QStringList isort_files;

  isort_files.clear ();

  do
    {
      isort_files << inputCursor.block ().text ();
    } while (inputCursor.movePosition (QTextCursor::NextBlock));


  //  Sort so we can remove dupes.

  isort_files.sort ();


  //  Remove dupes and place into input_files.

  QString name, prev_name = "";
  input_files.clear ();

  for (int32_t i = 0 ; i < isort_files.size () ; i++)
    {
      name = isort_files.at (i);

      if (name != prev_name)
        {
          input_files.append (name);
          prev_name = name;
        }
    }


  input_file_count = input_files.size ();


  //  Main processing loop

  TOF_HEADER_T                    tof_header;
  TOPO_OUTPUT_T                   tof;
  HOF_HEADER_T                    hof_header;
  HYDRO_OUTPUT_T                  hof;
  int32_t                         type;
  int32_t                         tmp_num_recs[2];
  char                            las_name[1024], string[1024];
  int32_t                         year, month, jday, mday;
  double                          min_x, max_x, min_y, max_y, min_z, max_z;
  time_t                          gps_tv_sec;
  long                            gps_tv_nsec;
  int64_t                         gps_start_time, start_sec;
  double                          lat, lon, z, time_offset;
  LASpoint                        laspoint;
  LASwriteOpener                  laswriteopener;
  LASwriter                       *laswriter;
  LASheader                       lasheader;
  LASvlr_key_entry                geo_keys[4];


  progress.obar->setRange (0, input_file_count * 100);


  //  Set the GPS start time (00:00:00 on 6 January 1980) in POSIX form.

  inv_cvtime (80, 6, 0, 0, 0.0, &gps_tv_sec, &gps_tv_nsec);
  gps_start_time = (int64_t) gps_tv_sec * 1000000;


  //  Loop through each input file.

  FILE *fp;


  for (int32_t i = 0 ; i < input_file_count ; i++)
    {
      tmp_num_recs[0] = tmp_num_recs[1] = 0;


      name = input_files.at (i);

      if (name.endsWith (".tof"))
        {
          type = 1;
        }
      else
        {
          type = 0;
        }


      strcpy (string, name.toLatin1 ());


      QString status;
      status.sprintf ("Processing file %d of %d : ", i + 1, input_file_count);
      status += QFileInfo (name).fileName ();

      QListWidgetItem *stat = new QListWidgetItem (status);

      progress.list->addItem (stat);
      progress.list->setCurrentItem (stat);
      progress.list->scrollToItem (stat);


      //  TOF file section

      if (type)
        {
          //  Try to open the input file.

          if ((fp = open_tof_file (string)) == NULL)
            {
              QMessageBox::warning (this, tr ("charts2LAS Open input files"), tr ("The file ") + name + 
                                    tr (" could not be opened.") + tr ("  The error message returned was:\n\n") +
                                    QString (strerror (errno)));
            }
          else
            {
              //  Read the header

              tof_read_header (fp, &tof_header);


	      //  Note, GPS time is ahead of UTC time by some number of leap seconds depending on the date of the survey.
	      //  The leap seconds that are relevant for CHARTS and/or CZMIL data are as follows
	      //
	      //  December 31 2005 23:59:59 - 1136073599 - 14 seconds ahead
	      //  December 31 2008 23:59:59 - 1230767999 - 15 seconds ahead
	      //  June 30 2012 23:59:59     - 1341100799 - 16 seconds ahead
	      //  June 30 2015 23:59:59     - 1435708799 - 17 seconds ahead

	      time_offset = 1000000000.0 - 13.0;

	      start_sec = tof_header.text.start_timestamp / 1000000;
	      if (start_sec > 1136073599) time_offset -= 1.0;
	      if (start_sec > 1230767999) time_offset -= 1.0;
	      if (start_sec > 1341100799) time_offset -= 1.0;
	      if (start_sec > 1435708799) time_offset -= 1.0;


              //  Check for files older than 10/07/2011.  At that point we changed the way we load TOF data into PFM
              //  so that we load first returns even if the second return is bad.

              uint8_t ver_dep_flag = NVFalse;
              int64_t hd_start_time;
              sscanf (tof_header.text.start_time, "%" PRId64, &hd_start_time);
              if (hd_start_time < 1317945600000000LL) ver_dep_flag = NVTrue;


              progress.fbar->reset ();

              progress.fbox->setTitle (QFileInfo (name).fileName ());

              progress.fbar->setRange (0, tof_header.text.number_shots);


              //  Check for compression.

              QString new_file;
              new_file = name.replace (".tof", "_t.las");
              if (compress) new_file = name.replace (".tof", "_t.laz");

              QFileInfo new_file_info (new_file);

              strcpy (las_name, new_file.toLatin1 ());


              lasheader.clean ();


              //  Fill the required header fields.

              memset (lasheader.system_identifier, 0, 32);
              memset (lasheader.generating_software, 0, 32);


              sprintf (lasheader.system_identifier, "CHARTS - system %02d", tof_header.text.ab_system_number);

              QString version = QString (VERSION).section ('-', 1, 2).simplified ();
              char ver[31];
              strcpy (ver, version.toLatin1 ());
              sprintf (lasheader.generating_software, "%s", ver);


              sscanf (tof_header.text.flight_date, "%04d%02d%02d", &year, &month, &mday);
              mday2jday (year, month, mday, &jday);


#ifdef _WIN32
  #if defined (__MINGW64__) || defined (__MINGW32__)
              putenv ((char *) "TZ=GMT");
              tzset ();
  #else
              _putenv ((char *) "TZ=GMT");
              _tzset ();
  #endif
#else
              putenv ((char *) "TZ=GMT");
              tzset ();
#endif


              lasheader.file_creation_day = jday;
              lasheader.file_creation_year = year;


              //  Header size for 1.2 is 227 (1.3 is 235 and 1.4 is 375).

              lasheader.header_size = 227;
              lasheader.offset_to_point_data = 227;


              lasheader.number_of_variable_length_records = 1;


              //  You can look these up in the GeoTIFF spec (appendices) basically they mean...


              //  Key 1

              //  GTModelTypeGeoKey (1024)

              geo_keys[0].key_id = 1024;


              //  Empty (in other words, no offset, we're putting the value here).

              geo_keys[0].tiff_tag_location = 0;
              geo_keys[0].count = 1;


              //  ModelTypeGeographic (2 - Geographic latitude-longitude System)

              geo_keys[0].value_offset = 2;



              //  Key 2

              //  GeographicTypeGeoKey (2048)

              geo_keys[1].key_id = 2048;
              geo_keys[1].tiff_tag_location = 0;
              geo_keys[1].count = 1;


              //  If one of the orthometric corrections was chosen then this is probably an Army Corps survey and 
              //  the horizontal datum is GCS_NAD83.  Otherwise, it's GCS_WGS_84.

              switch (ortho)
                {
                case 0:
                  geo_keys[1].value_offset = 4326;
                  break;

                case 1:
                case 2:
                case 3:
                  geo_keys[1].value_offset = 4269;
                  break;
                }



              //  Key 3

              //  VerticalCSTypeGeoKey (4096)

              geo_keys[2].key_id = 4096;
              geo_keys[2].tiff_tag_location = 0;
              geo_keys[2].count = 1;


              //  VertCS_WGS_84_ellipsoid (5030) or VertCS_North_American_Vertical_Datum_1988 (5103)

              switch (ortho)
                {
                case 0:
                  geo_keys[2].value_offset = 5030;
                  break;

                case 1:
                case 2:
                case 3:
                  geo_keys[2].value_offset = 5103;
                  break;
                }


              //  Key 4

              //  VerticalUnitsGeoKey (4099)

              geo_keys[3].key_id = 4099; // VerticalUnitsGeoKey
              geo_keys[3].tiff_tag_location = 0;
              geo_keys[3].count = 1;


              //  Linear_Meter (9001)

              geo_keys[3].value_offset = 9001;



              //  Set the LASF_Projection

              lasheader.set_geo_keys (4, geo_keys);


              lasheader.point_data_format = 1;
              lasheader.point_data_record_length = 28;

              for (int32_t pbr = 0 ; pbr < 5 ; pbr++) lasheader.number_of_points_by_return[pbr] = 0;

              lasheader.x_scale_factor = 0.0000001;
              lasheader.y_scale_factor = 0.0000001;
              lasheader.z_scale_factor = 0.001;

              lasheader.x_offset = 0.0;
              lasheader.y_offset = 0.0;
              lasheader.z_offset = 0.0;


              //  Set the global encoding slot in the header to 1 to store time as GPS time.

              lasheader.global_encoding = 1;


              //  Start out with bounds set to the entire world so LASlib won't bitch.

              lasheader.min_x = -180.0;
              lasheader.max_x = 180.0;
              lasheader.min_y = -90.0;
              lasheader.max_y = 90.0;

              lasheader.number_of_point_records = 0;

              laswriteopener.set_file_name (las_name);

              laswriter = laswriteopener.open (&lasheader);

              if (!laswriter)
                {
                  QMessageBox::critical (this, tr ("charts2LAS"), tr ("Could not create LAS file %1\n").arg (QString (las_name)));
                  exit (-1);
                }


              if (!laspoint.init (&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, &lasheader))
                {
                  QMessageBox::critical (this, tr ("charts2LAS"), tr ("Could not initialize LASPoint records for LAS file %1\n").arg (QString (las_name)));
                  exit (-1);
                }


              //  Set mins and maxes to stupid values so they'll be replaced when we load the points.

              max_x = max_y = max_z = -99999999999999.0;
              min_x = min_y = min_z = 99999999999999.0;


              int32_t prog_count = 0;


              //  Loop through the entire file reading each record.

              while (tof_read_record (fp, TOF_NEXT_RECORD, &tof))
                {

                  //  Pre-scan to get number of returns and count valid returns.

                  int32_t ret_count = 0;
                  uint8_t returns[2] = {NVTrue, NVTrue};

                  for (int32_t k = 0 ; k < 2 ; k++)
                    {
                      if (k)
                        {
                          //  A confidence value of 50 or below is bad.

                          if (tof.elevation_last == -998.0 || (tof.conf_last <= 50 && !invalid))
                            {
                              returns[k] = NVFalse;
                            }
                          else
                            {
                              tmp_num_recs[1]++;
                              ret_count++;
                            }
                        }
                      else
                        {
                          //  A confidence value of 50 or below is bad.
                          //  We don't load first returns if the last return was bad in files prior to 10/07/2011.

                          if (fabsf (tof.elevation_last - tof.elevation_first) <= 0.05 || tof.elevation_first == -998.0 ||
                              (tof.conf_first <= 50 && !invalid) || (ver_dep_flag && tof.elevation_last <= -998.0))
                            {
                              returns[k] = NVFalse;
                            }
                          else
                            {
                              tmp_num_recs[0]++;
                              ret_count++;
                            }
                        }
                    }


                  int32_t num_ret = 0;
                  for (int32_t k = 0 ; k < 2 ; k++)
                    {
                      //  If we got a return...

                      if (returns[k])
                        {
                          double lat, lon, z;


                          //  Build the LAS record


                          //  We're blowing off the edge_of_flightline 'cause we really don't care ;-)

                          laspoint.edge_of_flight_line = 0;


                          laspoint.scan_direction_flag = 1;
                          if (hof.scanner_azimuth < 0.0) laspoint.scan_direction_flag = 0;


                          uint8_t inv_flag = NVFalse;

                          if (k)
                            {
                              if (tof.conf_last <= 50) inv_flag = NVTrue;

                              num_ret++;
                              lat = tof.latitude_last;
                              lon = tof.longitude_last;
                              z = tof.elevation_last;
                              laspoint.intensity = tof.intensity_last;
                            }
                          else
                            {
                              if (tof.conf_first <= 50) inv_flag = NVTrue;

                              num_ret++;
                              lat = tof.latitude_first;
                              lon = tof.longitude_first;
                              z = tof.elevation_first;
                              laspoint.intensity = tof.intensity_first;
                            }


                          /*

                              Classification is not exactly per the LAS spec but it's pretty close.  We're having to use
                              one of the "reserved for ASPRS" values since there is no classification for bathy data.

                              Topo                       = 1
                              Ground                     = 2
                              Water surface              = 9
                              Bathy                      = 29

                          */

                          laspoint.classification = 1;


                          //  If the TOF classification_status has already been set to a value other than 0 we want use use that
                          //  value, as is (unless, of course, it's a LAS 1.4 value)...

                          if (tof.classification_status)
                            {
                              laspoint.classification = tof.classification_status;


                              //  These are LAS 1.4 classifications so we need to translate them to (almost) LAS 1.2 values

                              if (tof.classification_status == 40) laspoint.classification = 29;
                              if (tof.classification_status == 41) laspoint.classification = 9;
                              if (tof.classification_status == 42) laspoint.classification = 9;
                              if (tof.classification_status > 42) laspoint.classification = 1;
                            }


                          if (inv_flag)
                            {
                              laspoint.withheld_flag = 1;
                            }
                          else
                            {
                              laspoint.withheld_flag = 0;
                            }

                          laspoint.scan_angle_rank = NINT (tof.scanner_azimuth);


                          //  Convert UTC time to GPS time.

                          laspoint.gps_time = ((double) (tof.timestamp - gps_start_time) / 1000000.0) - time_offset;


                          //  If we are correcting to orthometric height... 

                          float value = -999.0;
                          switch (ortho)
                            {
                            case 1:
                              value = get_geoid03 (lat, lon);
                              break;

                            case 2:
                              value = get_geoid09 (lat, lon);
                              break;

                            case 3:
                              value = get_geoid12a (lat, lon);
                              break;

                            case 4:
                              value = get_geoid12b (lat, lon);
                              break;
                            }

                          if (value != -999.0) z -= value;


                          laspoint.X = NINT64 ((lon - lasheader.x_offset) / lasheader.x_scale_factor);
                          laspoint.Y = NINT64 ((lat - lasheader.y_offset) / lasheader.y_scale_factor);
                          laspoint.Z = NINT64 ((z - lasheader.z_offset) / lasheader.z_scale_factor);

                          laspoint.return_number = num_ret;
                          laspoint.number_of_returns = ret_count;


                          //  Check for min and max x, y, and z.

                          min_x = qMin (min_x, lon);
                          max_x = qMax (max_x, lon);
                          min_y = qMin (min_y, lat);
                          max_y = qMax (max_y, lat);
                          min_z = qMin (min_z, z);
                          max_z = qMax (max_z, z);


                          //  Increment the number of records counter in the header.

                          lasheader.number_of_point_records++;


                          //  Write out the LAS record

			  if (!laswriter->write_point (&laspoint))
                            {
                              QMessageBox::critical (this, tr ("charts2LAS"), tr ("Record %1 ").arg (lasheader.number_of_point_records) +
                                                     tr ("in file ") + name + tr (" could not be appended."));
                              exit (-1);
                            }
                        }
                    }

                  progress.fbar->setValue (prog_count);

                  progress.obar->setValue (i * 100 + (int32_t) (((float) prog_count / (float) tof_header.text.number_shots) * 100.0));
                  qApp->processEvents ();

                  prog_count++;
                }

              fclose (fp);

              progress.fbar->setValue (tof_header.text.number_shots);


              lasheader.min_x = min_x;
              lasheader.min_y = min_y;
              lasheader.min_z = min_z;
              lasheader.max_x = max_x;
              lasheader.max_y = max_y;
              lasheader.max_z = max_z;


              //  Save the number of points by return.

              lasheader.number_of_points_by_return[0] = tmp_num_recs[0];
              lasheader.number_of_points_by_return[1] = tmp_num_recs[1];


              //  Update the header with the min and max x, y, and z, and the number of records/returns.

              laswriter->update_header (&lasheader);


              laswriter->close();
            }
        }


      //  HOF file section

      else
        {
          //  Try to open the HOF file

          if ((fp = open_hof_file (string)) == NULL)
            {
              QMessageBox::warning (this, tr ("Open input files"), tr ("The file ") + name + 
                                    tr (" could not be opened.") + tr ("  The error message returned was:\n\n") +
                                    QString (strerror (errno)));
            }
          else
            {
              //  Read the header

              hof_read_header (fp, &hof_header);


	      //  Note, GPS time is ahead of UTC time by some number of leap seconds depending on the date of the survey.
	      //  The leap seconds that are relevant for CHARTS and/or CZMIL data are as follows
	      //
	      //  December 31 2005 23:59:59 - 1136073599 - 14 seconds ahead
	      //  December 31 2008 23:59:59 - 1230767999 - 15 seconds ahead
	      //  June 30 2012 23:59:59     - 1341100799 - 16 seconds ahead
	      //  June 30 2015 23:59:59     - 1435708799 - 17 seconds ahead

	      time_offset = 1000000000.0 - 13.0;

	      start_sec = hof_header.text.start_timestamp / 1000000;
	      if (start_sec > 1136073599) time_offset -= 1.0;
	      if (start_sec > 1230767999) time_offset -= 1.0;
	      if (start_sec > 1341100799) time_offset -= 1.0;
	      if (start_sec > 1435708799) time_offset -= 1.0;


              progress.fbar->reset ();

              progress.fbox->setTitle (QFileInfo (name).fileName ());

              progress.fbar->setRange (0, hof_header.text.number_shots);


              //  Check for compression.

              QString new_file = name.replace (".hof", "_h.las");
              if (compress) QString new_file = name.replace (".hof", "_h.laz");

              QFileInfo new_file_info (new_file);

              strcpy (las_name, new_file.toLatin1 ());


              lasheader.clean ();


              //  Fill the required header fields.

              memset (lasheader.system_identifier, 0, 32);
              memset (lasheader.generating_software, 0, 32);


              sprintf (lasheader.system_identifier, "CHARTS - system %02d", hof_header.text.ab_system_number);

              QString version = QString (VERSION).section ('-', 1, 2).simplified ();
              char ver[31];
              strcpy (ver, version.toLatin1 ());
              sprintf (lasheader.generating_software, "%s", ver);


              sscanf (hof_header.text.flight_date, "%04d%02d%02d", &year, &month, &mday);
              mday2jday (year, month, mday, &jday);


              putenv ((char *) "TZ=GMT");
              tzset ();


              lasheader.file_creation_day = jday;
              lasheader.file_creation_year = year + 1900;


              //  Header size for 1.2 is 227 (1.3 is 235 and 1.4 is 375).

              lasheader.header_size = 227;
              lasheader.offset_to_point_data = 227;



              //  You can look these up in the GeoTIFF spec (appendices) basically they mean...


              //  Key 1

              //  GTModelTypeGeoKey (1024)

              geo_keys[0].key_id = 1024;


              //  Empty (in other words, no offset, we're putting the value here).

              geo_keys[0].tiff_tag_location = 0;
              geo_keys[0].count = 1;


              //  ModelTypeGeographic  (2)   Geographic latitude-longitude System

              geo_keys[0].value_offset = 2;



              //  Key 2

              //  GeographicTypeGeoKey (2048) (ellipsoid)

              geo_keys[1].key_id = 2048;
              geo_keys[1].tiff_tag_location = 0;
              geo_keys[1].count = 1;


              //  GCS_WGS_84 (4326) or GCS_NAD_83 (4269)

              switch (ortho)
                {
                case 0:
                  geo_keys[1].value_offset = GCS_WGS_84;
                  break;

                case 1:
                case 2:
                case 3:
                  geo_keys[1].value_offset = GCS_NAD_83;
                  break;
                }



              //  Key 3

              //  VerticalUnitsGeoKey (4099)

              geo_keys[2].key_id = 4099; // VerticalUnitsGeoKey
              geo_keys[2].tiff_tag_location = 0;
              geo_keys[2].count = 1;


              //  Linear_Meter (9001)

              geo_keys[2].value_offset = 9001;


              //  Set projection

              lasheader.set_geo_keys (3, geo_keys);


              lasheader.point_data_format = 1;
              lasheader.point_data_record_length = 28;

              lasheader.number_of_variable_length_records = 1;

              for (int32_t pbr = 0 ; pbr < 5 ; pbr++) lasheader.number_of_points_by_return[pbr] = 0;

              lasheader.x_scale_factor = 0.0000001;
              lasheader.y_scale_factor = 0.0000001;
              lasheader.z_scale_factor = 0.001;

              lasheader.x_offset = 0.0;
              lasheader.y_offset = 0.0;
              lasheader.z_offset = 0.0;


              //  Set the global encoding slot in the header to 1 to store time as GPS time.

              lasheader.global_encoding = 1;


              //  Start out with bounds set to the entire world so LASlib won't bitch.

              lasheader.min_x = -180.0;
              lasheader.max_x = 180.0;
              lasheader.min_y = -90.0;
              lasheader.max_y = 90.0;

              lasheader.number_of_point_records = 0;

              laswriteopener.set_file_name (las_name);

              laswriter = laswriteopener.open (&lasheader);

              if (!laswriter)
                {
                  QMessageBox::critical (this, tr ("charts2LAS"), tr ("Could not create LAS file %1\n").arg (QString (las_name)));
                  exit (-1);
                }


              if (!laspoint.init (&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, &lasheader))
                {
                  QMessageBox::critical (this, tr ("charts2LAS"), tr ("Could not initialize LASPoint records for LAS file %1\n").arg (QString (las_name)));
                  exit (-1);
                }


              //  Set mins and maxes to stupid values so they'll be replaced when we load the points.

              max_x = max_y = max_z = -99999999999999.0;
              min_x = min_y = min_z = 99999999999999.0;


              int32_t prog_count = 0;


              //  Loop through the entire file reading each record.

              while (hof_read_record (fp, HOF_NEXT_RECORD, &hof))
                {
                  if (!(hof.status & AU_STATUS_DELETED_BIT) && (hof.abdc >= 70 || invalid) && hof.correct_depth != -998.0)
                    {
                      //  Build the LAS record


                      //  We're blowing off the edge_of_flightline 'cause we really don't care ;-)

                      laspoint.edge_of_flight_line = 0;


                      laspoint.scan_direction_flag = 1;
                      if (hof.scanner_azimuth < 0.0) laspoint.scan_direction_flag = 0;


                      tmp_num_recs[0]++;

                      lat = hof.latitude;
                      lon = hof.longitude;


                      laspoint.scan_angle_rank = NINT (hof.scanner_azimuth);


                      //  Convert UTC time to GPS time.

                      laspoint.gps_time = ((double) (hof.timestamp - gps_start_time) / 1000000.0) - time_offset;


                      //  If we are correcting to orthometric height... 

                      if (hof.data_type)
                        {
                          float value = -999.0;
                          switch (ortho)
                            {
                            case 1:
                              value = get_geoid03 (lat, lon);
                              break;

                            case 2:
                              value = get_geoid09 (lat, lon);
                              break;

                            case 3:
                              value = get_geoid12a (lat, lon);
                              break;

                            case 4:
                              value = get_geoid12b (lat, lon);
                              break;
                            }

                          if (value != -999.0)
                            {
                              z = hof.correct_depth - value;
                            }
                          else
                            {
                              z = hof.correct_depth;
                            }
                        }
                      else
                        {
                          z = hof.correct_depth;
                        }

                      /*

                          Classification is not exactly per the LAS spec but it's pretty close.  We're having to use
                          one of the "reserved for ASPRS" values since there is no classification for bathy data.

                          Topo                       = 1
                          Ground                     = 2
                          Water surface              = 9
                          Bathy                      = 29

                      */

                      laspoint.classification = 29;


                      //  If the HOF classification_status has already been set to a value other than 0 we want use use that
                      //  value, as is (unless, of course, it's a LAS 1.4 value)...

                      if (hof.classification_status)
                        {
                          laspoint.classification = hof.classification_status;


                          //  These are LAS 1.4 classifications so we need to translate them to (almost) LAS 1.2 values

                          if (hof.classification_status == 40) laspoint.classification = 29;
                          if (hof.classification_status == 41) laspoint.classification = 9;
                          if (hof.classification_status == 42) laspoint.classification = 9;
                          if (hof.classification_status > 42) laspoint.classification = 1;
                        }


                      if (hof.abdc < 70)
                        {
                          laspoint.withheld_flag = 1;
                        }
                      else
                        {
                          laspoint.withheld_flag = 0;
                        }

                      laspoint.X = NINT64 ((lon - lasheader.x_offset) / lasheader.x_scale_factor);
                      laspoint.Y = NINT64 ((lat - lasheader.y_offset) / lasheader.y_scale_factor);
                      laspoint.Z = NINT64 ((z - lasheader.z_offset) / lasheader.z_scale_factor);

                      laspoint.return_number = 1;
                      laspoint.number_of_returns = 1;


                      //  Check for min and max x, y, and z.

                      min_x = qMin (min_x, lon);
                      max_x = qMax (max_x, lon);
                      min_y = qMin (min_y, lat);
                      max_y = qMax (max_y, lat);
                      min_z = qMin (min_z, z);
                      max_z = qMax (max_z, z);


                      //  Increment the number of records counter in the header.

                      lasheader.number_of_point_records++;


                      //  Write out the LAS record

                      if (!laswriter->write_point (&laspoint))
                        {
                          QMessageBox::critical (this, tr ("charts2LAS"), tr ("Record %1 ").arg (lasheader.number_of_point_records) +
                                                 tr ("in file ") + name + tr (" could not be appended."));
                          exit (-1);
                        }
                    }


                  progress.fbar->setValue (prog_count);

                  progress.obar->setValue (i * 100 + (int32_t) (((float) prog_count / (float) hof_header.text.number_shots) * 100.0));
                  qApp->processEvents ();

                  prog_count++;
                }

              fclose (fp);

              progress.fbar->setValue (hof_header.text.number_shots);


              lasheader.min_x = min_x;
              lasheader.min_y = min_y;
              lasheader.min_z = min_z;
              lasheader.max_x = max_x;
              lasheader.max_y = max_y;
              lasheader.max_z = max_z;


              //  Save the number of points by return.

              lasheader.number_of_points_by_return[0] = tmp_num_recs[0];
              lasheader.number_of_points_by_return[1] = tmp_num_recs[1];


              //  Update the header with the min and max x, y, and z, and the number of records/returns.

              laswriter->update_header (&lasheader);


              laswriter->close();
            }
        }
    }

  progress.obar->setValue (input_file_count * 100);


  button (QWizard::FinishButton)->setEnabled (true);
  button (QWizard::CancelButton)->setEnabled (false);


  QApplication::restoreOverrideCursor ();
  qApp->processEvents ();


  progress.list->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Processing complete, press Finish to exit."));

  progress.list->addItem (cur);
  progress.list->setCurrentItem (cur);
  progress.list->scrollToItem (cur);
}



//  Get the users defaults.

void
charts2LAS::envin ()
{
  //  We need to get the font from the global settings.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  font = QApplication::font ();

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  QString defaultFont = font.toString ();
  QString fontString = settings2.value (QString ("ABE map GUI font"), defaultFont).toString ();
  font.fromString (fontString);


  settings2.endGroup ();


  double saved_version = 1.0;


  // Set defaults so that if keys don't exist the parameters are defined

  inputFileFilter = tr ("TOF (*.tof)");
  window_x = 0;
  window_y = 0;
  window_width = 900;
  window_height = 500;
  invalid = NVFalse;
  ortho = 0;
  compress = NVFalse;
  input_dir = ".";


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/charts2LAS.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/charts2LAS.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("charts2LAS");

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  inputFileFilter = settings.value (tr ("input file filter"), inputFileFilter).toString ();

  window_width = settings.value (tr ("width"), window_width).toInt ();
  window_height = settings.value (tr ("height"), window_height).toInt ();
  window_x = settings.value (tr ("x position"), window_x).toInt ();
  window_y = settings.value (tr ("y position"), window_y).toInt ();

  invalid = settings.value (tr ("include invalid data"), invalid).toBool ();

  ortho = settings.value (tr ("orthometric correction"), ortho).toInt ();

  compress = settings.value (tr ("compress"), compress).toBool ();

  input_dir = settings.value (tr ("input directory"), input_dir).toString ();

  settings.endGroup ();
}




//  Save the users defaults.

void
charts2LAS::envout ()
{
  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/charts2LAS.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/charts2LAS.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("charts2LAS");


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("input file filter"), inputFileFilter);

  settings.setValue (tr ("width"), window_width);
  settings.setValue (tr ("height"), window_height);
  settings.setValue (tr ("x position"), window_x);
  settings.setValue (tr ("y position"), window_y);

  settings.setValue (tr ("include invalid data"), invalid);

  settings.setValue (tr ("orthometric correction"), ortho);

  settings.setValue (tr ("compress"), compress);

  settings.setValue (tr ("input directory"), input_dir);

  settings.endGroup ();
}



void 
charts2LAS::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}
