
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



#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (QWidget *parent, uint8_t in, uint8_t *orth, uint8_t cmp):
  QWizardPage (parent)
{
  invalid = in;
  ortho = orth;
  compress = cmp;


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/charts2LASWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("The charts2LAS program is used to convert CHARTS HOF and TOF files to LAS format.  "
                                  "Click the Next button to go to the input file page.  Context sensitive help is available "
                                  "by clicking on the Help button and then clicking, with the Question Arrow cursor, on the "
                                  "field of interest."));

  label->setWordWrap(true);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->addWidget (label);
  vbox->addStretch (10);


  QGroupBox *orthoBox = new QGroupBox (tr ("Orthometric correction"), this);
  QVBoxLayout *orthoBoxLayout = new QVBoxLayout;
  orthoBox->setLayout (orthoBoxLayout);
  orthoBoxLayout->setSpacing (10);

  QRadioButton *none = new QRadioButton (tr ("None"));
  none->setToolTip (tr ("If checked, data will not be orthometrically corrected"));
  QRadioButton *e03 = new QRadioButton (tr ("GEOID03"));
  e03->setToolTip (tr ("If checked, data will be orthometrically corrected using NGS GEOID03"));
  e03->setWhatsThis (ortho03Text);
  QRadioButton *e09 = new QRadioButton (tr ("GEOID09"));
  e09->setToolTip (tr ("If checked, data will be orthometrically corrected using NGS GEOID09"));
  e09->setWhatsThis (ortho09Text);
  QRadioButton *e12a = new QRadioButton (tr ("GEOID12a"));
  e12a->setToolTip (tr ("If checked, data will be orthometrically corrected using NGS GEOID12a"));
  e12a->setWhatsThis (ortho12aText);
  QRadioButton *e12b = new QRadioButton (tr ("GEOID12b"));
  e12b->setToolTip (tr ("If checked, data will be orthometrically corrected using NGS GEOID12b"));
  e12b->setWhatsThis (ortho12bText);

  QButtonGroup *bGrp = new QButtonGroup (this);
  bGrp->setExclusive (true);
  connect (bGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotOrthoClicked (int)));

  bGrp->addButton (none, 0);
  bGrp->addButton (e03, 1);
  bGrp->addButton (e09, 2);
  bGrp->addButton (e12a, 3);
  bGrp->addButton (e12b, 4);

  orthoBoxLayout->addWidget (none);
  orthoBoxLayout->addWidget (e03);
  orthoBoxLayout->addWidget (e09);
  orthoBoxLayout->addWidget (e12a);
  orthoBoxLayout->addWidget (e12b);

  orthoBoxLayout->addStretch (1);

  bGrp->button (*ortho)->setChecked (true);

  vbox->addWidget (orthoBox);


  //  Check for the availability of the geoid files and set the help and button states accordingly.

  int8_t g03, g09, g12a, g12b;

  g03 = check_geoid03 ();
  g09 = check_geoid09 ();
  g12a = check_geoid12a ();
  g12b = check_geoid12b ();

  switch (g03)
    {
    case 0:
      e03->setToolTip (tr ("If checked, data will be orthometrically corrected using NGS GEOID03"));
      e03->setWhatsThis (ortho03Text);
      break;

    case -1:
      e03->setToolTip (tr ("NGS GEOID03 data is not available because the ABE_DATA environment variable is not set"));
      e03->setWhatsThis (tr ("NGS GEOID03 data is not available because the ABE_DATA environment variable is not set"));
      e03->setEnabled (false);
      break;

    case -2:
      e03->setToolTip (tr ("NGS GEOID03 data is not available because one or more of the geoid03 files is missing from the ABE_DATA/geoid_data folder"));
      e03->setWhatsThis (tr ("NGS GEOID03 data is not available because one or more of the geoid03 files is missing from the ABE_DATA/geoid_data folder"));
      e03->setEnabled (false);
      break;
    }

  switch (g09)
    {
    case 0:
      e09->setToolTip (tr ("If checked, data will be orthometrically corrected using NGS GEOID09"));
      e09->setWhatsThis (ortho09Text);
      break;

    case -1:
      e09->setToolTip (tr ("NGS GEOID09 data is not available because the ABE_DATA environment variable is not set"));
      e09->setWhatsThis (tr ("NGS GEOID09 data is not available because the ABE_DATA environment variable is not set"));
      e09->setEnabled (false);
      break;

    case -2:
      e09->setToolTip (tr ("NGS GEOID09 data is not available because one or more of the geoid09 files is missing from the ABE_DATA/geoid_data folder"));
      e09->setWhatsThis (tr ("NGS GEOID09 data is not available because one or more of the geoid09 files is missing from the ABE_DATA/geoid_data folder"));
      e09->setEnabled (false);
      break;
    }

  switch (g12a)
    {
    case 0:
      e12a->setToolTip (tr ("If checked, data will be orthometrically corrected using NGS GEOID12a"));
      e12a->setWhatsThis (ortho12aText);
      break;

    case -1:
      e12a->setToolTip (tr ("NGS GEOID12a data is not available because the ABE_DATA environment variable is not set"));
      e12a->setWhatsThis (tr ("NGS GEOID12a data is not available because the ABE_DATA environment variable is not set"));
      e12a->setEnabled (false);
      break;

    case -2:
      e12a->setToolTip (tr ("NGS GEOID12a data is not available because one or more of the geoid12a files is missing from the ABE_DATA/geoid_data folder"));
      e12a->setWhatsThis (tr ("NGS GEOID12a data is not available because one or more of the geoid12a files is missing from the ABE_DATA/geoid_data folder"));
      e12a->setEnabled (false);
      break;
    }

  switch (g12b)
    {
    case 0:
      e12b->setToolTip (tr ("If checked, data will be orthometrically corrected using NGS GEOID12b"));
      e12b->setWhatsThis (ortho12bText);
      break;

    case -1:
      e12b->setToolTip (tr ("NGS GEOID12b data is not available because the ABE_DATA environment variable is not set"));
      e12b->setWhatsThis (tr ("NGS GEOID12b data is not available because the ABE_DATA environment variable is not set"));
      e12b->setEnabled (false);
      break;

    case -2:
      e12b->setToolTip (tr ("NGS GEOID12b data is not available because one or more of the geoid12b files is missing from the ABE_DATA/geoid_data folder"));
      e12b->setWhatsThis (tr ("NGS GEOID12b data is not available because one or more of the geoid12b files is missing from the ABE_DATA/geoid_data folder"));
      e12b->setEnabled (false);
      break;
    }


  QGroupBox *oBox = new QGroupBox (this);
  QHBoxLayout *oBoxLayout = new QHBoxLayout;
  oBox->setLayout (oBoxLayout);
  oBoxLayout->setSpacing (10);


  QGroupBox *iBox = new QGroupBox (tr ("Invalid data"), this);
  QHBoxLayout *iBoxLayout = new QHBoxLayout;
  iBox->setLayout (iBoxLayout);
  iBoxLayout->setSpacing (10);


  oBoxLayout->addWidget (iBox);


  inv = new QCheckBox (this);
  inv->setToolTip (tr ("If checked, include invalid data in output LAS file"));
  inv->setWhatsThis (invText);
  inv->setChecked (invalid);
  iBoxLayout->addWidget (inv);


  QGroupBox *cBox = new QGroupBox (tr ("Compress"), this);
  QHBoxLayout *cBoxLayout = new QHBoxLayout;
  cBox->setLayout (cBoxLayout);
  cBoxLayout->setSpacing (10);
  cBox->setToolTip (tr ("If checked, compress output files"));
  cBox->setWhatsThis (compText);


  oBoxLayout->addWidget (cBox);


  comp = new QCheckBox (this);
  comp->setToolTip (tr ("If checked, compress output files"));
  comp->setWhatsThis (compText);
  comp->setChecked (compress);
  cBoxLayout->addWidget (comp);


  vbox->addWidget (oBox);


  registerField ("invalid", inv);
  registerField ("compress", comp);
}



void startPage::slotOrthoClicked (int id)
{
  *ortho = id;
}
