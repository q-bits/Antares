#pragma once
using namespace System;

namespace Antares {
public ref class lang
{
public:
static String^ tb_up;
static String^ tb_refresh;
static String^ tb_delete;
static String^ tb_new;
static String^ tb_cut;
static String^ tb_paste;
static String^ tb_info;
static String^ tb_settings;
static String^ tb_turbo_mode;
static String^ tt_up;
static String^ tt_refresh;
static String^ tt_delete;
static String^ tt_new;
static String^ tt_cut;
static String^ tt_paste;
static String^ tt_info;
static String^ tt_settings;
static String^ cb_move;
static String^ st_not_connected;
static String^ st_in_use;
static String^ st_wrong_driver;
static String^ st_toppy;
static String^ st_toppy2;
static String^ st_local;
static String^ st_free;
static String^ st_total;
static String^ st_my_computer;
static String^ st_denied;
static String^ st_error;
static String^ st_pc_selected;
static String^ st_pc_selected_with_folder;
static String^ st_pc_selected_with_folders;
static String^ st_pvr_selected;
static String^ st_pvr_selected_with_folder;
static String^ st_pvr_selected_with_folders;
static String^ h_name;
static String^ h_size;
static String^ h_type;
static String^ h_date;
static String^ h_channel;
static String^ h_description;
static String^ u_bytes;
static String^ u_kb;
static String^ u_mb;
static String^ u_gb;
static String^ u_tb;
static String^ u_mb_per_sec;
static String^ u_hours;
static String^ u_minutes;
static String^ c_no_space_pc;
static String^ c_no_space_pvr;
static String^ c_error;
static String^ c_checking;
static String^ c_folder_error;
static String^ c_bad_location;
static String^ c_error_writing;
static String^ c_no_access_pvr;
static String^ c_no_access_pvr_plural;
static String^ c_no_access_pc;
static String^ c_no_access_pc_plural;
static String^ c_folder_file_clash;
static String^ b_ok;
static String^ b_cancel;
static String^ b_yes;
static String^ b_no;
static String^ b_close;
static String^ b_cancel_transfer;
static String^ c_title1_copy;
static String^ c_title1_move;
static String^ c_title1b_copy;
static String^ c_title1b_move;
static String^ c_title2_to_pc;
static String^ c_title2_to_pvr;
static String^ c_finding;
static String^ c_completion;
static String^ c_sleep;
static String^ c_hibernate;
static String^ c_shutdown;
static String^ c_turbo_changing;
static String^ o_exist;
static String^ o_exist_plural;
static String^ o_delete_pvr;
static String^ o_delete_pvr_plural;
static String^ o_delete_pc;
static String^ o_delete_pc_plural;
static String^ o_correct_plural;
static String^ o_correct;
static String^ o_undersized_plural;
static String^ o_undersized;
static String^ o_oversized_plural;
static String^ o_oversized;
static String^ o_skip;
static String^ o_overwrite;
static String^ o_resume;
static String^ o_skip_r;
static String^ f_title;
static String^ f_warning;
static String^ f_required;
static String^ f_available;
static String^ f_proceed;
static String^ d_title;
static String^ d_folder;
static String^ d_delete;
static String^ d_delete_plural;
static String^ d_error;
static String^ m_rename_error;
static String^ m_folder_error;
static String^ m_paste_error;
static String^ m_new_folder_name;
static String^ cm_copy_pc;
static String^ cm_info;
static String^ cm_move_pc;
static String^ cm_select_all;
static String^ cm_copy_pvr;
static String^ cm_move_pvr;
static String^ cm_explorer;
static String^ cm_rename;
static String^ cm_firmware;
static String^ s_title;
static String^ s_choose;
static String^ s_rescale;
static String^ s_idle;
static String^ s_language;
static String^ p_wintitle;
static String^ p_title;
static String^ p_proglen;
static String^ p_reclen;
static void set_en_au(void){
tb_up                       = "Up";
tb_refresh                  = "Refresh";
tb_delete                   = "Delete";
tb_new                      = "New";
tb_cut                      = "Cut";
tb_paste                    = "Paste";
tb_info                     = "Info.";
tb_settings                 = "Settings";
tb_turbo_mode               = "Turbo mode";
tt_up                       = "Up Folder";
tt_refresh                  = "Refresh";
tt_delete                   = "Delete";
tt_new                      = "New Folder";
tt_cut                      = "Cut";
tt_paste                    = "Paste";
tt_info                     = "Show program information";
tt_settings                 = "Change Antares' settings";
cb_move                     = "Move";
st_not_connected            = "PVR: Device not connected";
st_in_use                   = "PVR: Error -- already in use.";
st_wrong_driver             = "PVR: Error -- wrong driver installed.";
st_toppy                    = "Topfield device";
st_toppy2                   = "Topfield second device";
st_local                    = "Local Disk";
st_free                     = "Free";
st_total                    = "Total";
st_my_computer              = "My Computer";
st_denied                   = "Access denied:";
st_error                    = "Error";
st_pc_selected              = "Selected {0} files on PC  ( {1} )";
st_pc_selected_with_folder  = "Selected {0} files on PC  ( {1} )   and   1 folder (size unknown)";
st_pc_selected_with_folders = "Selected {0} files on PC  ( {1} )   and   {2} folders (size unknown)";
st_pvr_selected             = "Selected {0} files on PVR  ( {1} )";
st_pvr_selected_with_folder = "Selected {0} files on PVR  ( {1} )   and   1 folder (size unknown)";
st_pvr_selected_with_folders= "Selected {0} files on PVR  ( {1} )   and   {2} folders (size unknown)";
h_name                      = "Name";
h_size                      = "Size";
h_type                      = "Type";
h_date                      = "Date";
h_channel                   = "Channel";
h_description               = "Description";
u_bytes                     = "bytes";
u_kb                        = "KB";
u_mb                        = "MB";
u_gb                        = "GB";
u_tb                        = "TB";
u_mb_per_sec                = "MB/s";
u_hours                     = "hr";
u_minutes                   = "min";
c_no_space_pc               = "NO SPACE LEFT ON PC. Retrying . . .";
c_no_space_pvr              = "NO SPACE LEFT ON PVR. Retrying . . .";
c_error                     = "ERROR CONNECTING TO THE PVR. Retrying . . .";
c_checking                  = "Checking free space on PVR...";
c_folder_error              = "The folder {0} could not be created. Aborting transfer.";
c_bad_location              = "Antares cannot save the file to the location you chose. Please select another location and try again.";
c_error_writing             = "An error occurred writing the file to your computer.";
c_no_access_pvr             = "The following file could not be accessed on the PVR:";
c_no_access_pvr_plural      = "The following files could not be accessed on the PVR:";
c_no_access_pc              = "The following file could not be accessed on the PC:";
c_no_access_pc_plural       = "The following files could not be accessed on the PC:";
c_folder_file_clash         = "The folder {0} could not be created because there exists a file of the same name.";
b_ok                        = "OK";
b_cancel                    = "Cancel";
b_yes                       = "Yes";
b_no                        = "No";
b_close                     = "Close";
b_cancel_transfer           = "Cancel transfer";
c_title1_copy               = "Copying File(s)";
c_title1_move               = "Moving File(s)";
c_title1b_copy              = "Copying File {0} of {1}";
c_title1b_move              = "Moving File {0} of {1}";
c_title2_to_pc              = "[PVR --> PC]";
c_title2_to_pvr             = "[PC --> PVR]";
c_finding                   = "Finding files...";
c_completion                = "On completion:";
c_sleep                     = "Sleep";
c_hibernate                 = "Hibernate";
c_shutdown                  = "Shutdown";
c_turbo_changing            = "Turbo mode [Changing...]";
o_exist                     = "A file with this name already exists";
o_exist_plural              = "Files with these names already exist";
o_delete_pvr                = "Delete the PVR copy";
o_delete_pvr_plural         = "Delete the PVR copies";
o_delete_pc                 = "Delete the PC copy";
o_delete_pc_plural          = "Delete the PC copies";
o_correct_plural            = "Files have correct size";
o_correct                   = "File has correct size";
o_undersized_plural         = "Undersized files";
o_undersized                = "Undersized file";
o_oversized_plural          = "These existing files are larger!";
o_oversized                 = "This existing file is larger!";
o_skip                      = "Skip";
o_overwrite                 = "Overwrite";
o_resume                    = "Resume (recommended)";
o_skip_r                    = "Skip (recommended)";
f_title                     = "Low space";
f_warning                   = "Warning: not enough free space!";
f_required                  = "Required:";
f_available                 = "Available:";
f_proceed                   = "Proceed anyway?";
d_title                     = "Delete Confirmation";
d_folder                    = "[Folder -- Contents will be deleted!!!]";
d_delete                    = "Delete the following item?";
d_delete_plural             = "Delete the following items?";
d_error                     = "An error occurred while deleting.";
m_rename_error              = "An error occurred during rename.";
m_folder_error              = "Error creating new folder.";
m_paste_error               = "Cannot paste to this location, since it is inside a folder being moved.";
m_new_folder_name           = "New Folder";
cm_copy_pc                  = "Copy to PC";
cm_info                     = "Show program information";
cm_move_pc                  = "Move to PC";
cm_select_all               = "Select all";
cm_copy_pvr                 = "Copy to PVR";
cm_move_pvr                 = "Move to PVR";
cm_explorer                 = "Show in Explorer";
cm_rename                   = "Rename";
cm_firmware                 = "Install firmware to PVR";
s_title                     = "Antares Settings";
s_choose                    = "Choose Columns";
s_rescale                   = "Rescale column widths automatically";
s_idle                      = "During a transfer, prevent Windows from automatically going to sleep when idle";
s_language                  = "Language selection:";
p_wintitle                  = "Program Information";
p_title                     = "Title:";
p_proglen                   = "Program length:";
p_reclen                    = "Recorded length:";
}
static void set_fi(void){
tb_up                       = "Yl�s";
tb_refresh                  = "P�ivit�";
tb_delete                   = "Poista";
tb_new                      = "Uusi";
tb_cut                      = "Leikkaa";
tb_paste                    = "Liit�";
tb_info                     = "Info";
tb_settings                 = "Asetukset";
tb_turbo_mode               = "Turbo-tila";
tt_up                       = "Yl�kansio";
tt_refresh                  = "P�ivit�";
tt_delete                   = "Poista";
tt_new                      = "Uusi kansio";
tt_cut                      = "Leikkaa";
tt_paste                    = "Liit�";
tt_info                     = "N�yt� ohjelmatiedot";
tt_settings                 = "Muuta Antares-asetuksia";
cb_move                     = "Siirr�";
st_not_connected            = "PVR: Laite ei yhdistetty";
st_in_use                   = "PVR: Virhe -- jo k�yt�ss�";
st_wrong_driver             = "PVR: Virhe -- v��r� ohjain asennettu";
st_toppy                    = "Topfield-laite";
st_toppy2                   = "Toinen Topfield-laite";
st_local                    = "Paikallinen levy";
st_free                     = "Vapaa";
st_total                    = "Yhteens�";
st_my_computer              = "Oma tietokone";
st_denied                   = "P��sy estetty:";
st_error                    = "Virhe";
st_pc_selected              = "Valittu {0} tiedostoa PC:ll� ( {1} )";
st_pc_selected_with_folder  = "Valittu {0} tiedostoa PC:ll� ( {1} ) ja 1 kansio (koko tuntematon)";
st_pc_selected_with_folders = "Valittu {0} tiedostoa PC:ll� ( {1} ) ja {2} kansiota (koko tuntematon)";
st_pvr_selected             = "Valittu {0} tiedostoa PVR:ll� ( {1} )";
st_pvr_selected_with_folder = "Valittu {0} tiedostoa PVR:ll�  ( {1} ) ja 1 kansio (koko tuntematon)";
st_pvr_selected_with_folders= "Valittu {0} tiedostoa PVR:ll�  ( {1} ) ja {2} kansiota (koko tuntematon)";
h_name                      = "Nimi";
h_size                      = "Koko";
h_type                      = "Tyyppi";
h_date                      = "P�iv�ys";
h_channel                   = "Kanava";
h_description               = "Kuvaus";
u_bytes                     = "tavua";
u_kb                        = "KB";
u_mb                        = "MB";
u_gb                        = "GB";
u_tb                        = "TB";
u_mb_per_sec                = "MB/s";
u_hours                     = "t";
u_minutes                   = "min";
c_no_space_pc               = "PC:ll� ei tilaa. Uusi yritys...";
c_no_space_pvr              = "PVR:ll� ei tilaa. Uusi yritys...";
c_error                     = "Virhe yhdistett�ess�. Uusi yritys...";
c_checking                  = "Vapaata levytilaa PVR:ss� tarkistetaan...";
c_folder_error              = "Kansiota {0} ei voitu luoda. Siirto hyl�t��n.";
c_bad_location              = "Antares ei voi tallentaa valitsemaasi kohteeseen. Valitse toinen kohde ja yrit� uudelleen.";
c_error_writing             = "Virhe kirjoitettaessa tiedostoa tietokoneellesi.";
c_no_access_pvr             = "Seuraavaa tiedostoa ei voitu avata PVR:lt�:";
c_no_access_pvr_plural      = "Seuraavia tiedostoja ei voitu avata PVR:lt�:";
c_no_access_pc              = "Seuraavaa tiedostoa ei voitu avata PC:lt�:";
c_no_access_pc_plural       = "Seuraavia tiedostoja ei voitu avata PC:lt�:";
c_folder_file_clash         = "Kansiota {0} ei voida luoda, koska samanniminen tiedosto on jo olemassa.";
b_ok                        = "OK";
b_cancel                    = "Peruuta";
b_yes                       = "Kyll�";
b_no                        = "Ei";
b_close                     = "Sulje";
b_cancel_transfer           = "Peruuta siirto";
c_title1_copy               = "Kopioidaan tiedosto(ja)";
c_title1_move               = "Siirret��n tiedosto(ja)";
c_title1b_copy              = "Kopioidaan tiedostoa {0} / {1}";
c_title1b_move              = "Siirret��n tiedostoa {0} / {1}";
c_title2_to_pc              = "[PVR --> PC]";
c_title2_to_pvr             = "[PC --> PVR]";
c_finding                   = "Etsit��n tiedostoja...";
c_completion                = "Toiminnon valmistuessa:";
c_sleep                     = "Lepotila";
c_hibernate                 = "Horrostila";
c_shutdown                  = "Sammuta tietokone";
c_turbo_changing            = "Turbo-tila [Vaihdetaan...]";
o_exist                     = "Samanniminen tiedosto on jo olemassa";
o_exist_plural              = "Samannimisi� tiedostoja on jo olemassa";
o_delete_pvr                = "Poista PVR-kopio";
o_delete_pvr_plural         = "Poista PVR-kopiot";
o_delete_pc                 = "Poista PC-kopio";
o_delete_pc_plural          = "Poista PC-kopiot";
o_correct_plural            = "Tiedostojen koot ovat oikein";
o_correct                   = "Tiedoston koko on oikein";
o_undersized_plural         = "Tiedostot lyhyempi�";
o_undersized                = "Tiedosto lyhyempi";
o_oversized_plural          = "Olemassaolevat tiedostot ovat suurempia!";
o_oversized                 = "Olemassaoleva tiedosto on suurempi!";
o_skip                      = "Ohita";
o_overwrite                 = "Korvaa";
o_resume                    = "Jatka (suositellaan)";
o_skip_r                    = "Ohita (suositellaan)";
f_title                     = "V�h�n levytilaa";
f_warning                   = "Varoitus: ei tarpeeksi levytilaa!";
f_required                  = "Tarvitaan:";
f_available                 = "K�ytett�viss�:";
f_proceed                   = "Jatkatko kuitenkin?";
d_title                     = "Poiston vahvistus";
d_folder                    = "[Kansio -- Sis�lt� tuhotaan!!!]";
d_delete                    = "Poistatko seuraavan ohjelman?";
d_delete_plural             = "Poistatko seuraavat ohjelmat?";
d_error                     = "Virhe poistettaessa";
m_rename_error              = "Virhe uudelleennime�misess�";
m_folder_error              = "Virhe kansiota luotaessa";
m_paste_error               = "Ei voida liitt�� t�h�n kohteeseen, koska se on siirrett�v�ss� kansiossa.";
m_new_folder_name           = "Uusi kansio";
cm_copy_pc                  = "Kopioi PC:lle";
cm_info                     = "N�yt� ohjelmatiedot";
cm_move_pc                  = "Siirr� PC:lle";
cm_select_all               = "Valitse kaikki";
cm_copy_pvr                 = "Kopioi PVR:lle";
cm_move_pvr                 = "Siirr� PVR:lle";
cm_explorer                 = "N�yt� resurssienhallinnassa";
cm_rename                   = "Nime� uudelleen";
cm_firmware                 = "Asenna firmware";
s_title                     = "Antares-asetukset";
s_choose                    = "Valitse sarakkeet";
s_rescale                   = "S��d� sarakeleveydet automaattisesti";
s_idle                      = "Siirron aikana est� Windowsia menem�st� lepotilaan";
s_language                  = "Kielivalinta:";
p_wintitle                  = "Ohjelmatiedot";
p_title                     = "Otsikko:";
p_proglen                   = "Ohjelman pituus:";
p_reclen                    = "Tallennettu pituus:";
}
static void set_de(void){
tb_up                       = "Hoch";
tb_refresh                  = "Neu laden";
tb_delete                   = "Entfernen";
tb_new                      = "Neu";
tb_cut                      = "Ausschneiden";
tb_paste                    = "Einf�gen";
tb_info                     = "Info.";
tb_settings                 = "Einstellungen";
tb_turbo_mode               = "Turbo-Modus";
tt_up                       = "Eine Ordnerebene h�her";
tt_refresh                  = "Neu laden";
tt_delete                   = "Entfernen";
tt_new                      = "Neuer Ordner";
tt_cut                      = "Ausschneiden";
tt_paste                    = "Einf�gen";
tt_info                     = "Programminformationen anzeigen";
tt_settings                 = "Wechseln zu Einstellungen";
cb_move                     = "Move";
st_not_connected            = "PVR: Laufwerk nicht verbunden";
st_in_use                   = "PVR: Fehler: In Benutzung";
st_wrong_driver             = "PVR: Fehler: falscher Treiber installiert";
st_toppy                    = "Topfield Laufwerk";
st_toppy2                   = "Topfield 2. Laufwerk";
st_local                    = "Lokale Festplatte";
st_free                     = "Frei";
st_total                    = "Insgesamt";
st_my_computer              = "Mein Computer";
st_denied                   = "Zugriff verweigert:";
st_error                    = "Error";
st_pc_selected              = "Selected {0} files on PC  ( {1} )";
st_pc_selected_with_folder  = "Selected {0} files on PC  ( {1} )   and   1 folder (size unknown)";
st_pc_selected_with_folders = "Selected {0} files on PC  ( {1} )   and   {2} folders (size unknown)";
st_pvr_selected             = "Selected {0} files on PVR  ( {1} )";
st_pvr_selected_with_folder = "Selected {0} files on PVR  ( {1} )   and   1 folder (size unknown)";
st_pvr_selected_with_folders= "Selected {0} files on PVR  ( {1} )   and   {2} folders (size unknown)";
h_name                      = "Name";
h_size                      = "Gr��e";
h_type                      = "Typ";
h_date                      = "Datum";
h_channel                   = "Sender";
h_description               = "Beschreibung";
u_bytes                     = "bytes";
u_kb                        = "KB";
u_mb                        = "MB";
u_gb                        = "GB";
u_tb                        = "TB";
u_mb_per_sec                = "MB/s";
u_hours                     = "hr";
u_minutes                   = "min";
c_no_space_pc               = "NO SPACE LEFT ON PC. Retrying . . .";
c_no_space_pvr              = "NO SPACE LEFT ON PVR. Retrying . . .";
c_error                     = "ERROR CONNECTING TO THE PVR. Retrying . . .";
c_checking                  = "Checking free space on PVR...";
c_folder_error              = "The folder {0} could not be created. Aborting transfer.";
c_bad_location              = "Antares cannot save the file to the location you chose. Please select another location and try again.";
c_error_writing             = "An error occurred writing the file to your computer.";
c_no_access_pvr             = "The following file could not be accessed on the PVR:";
c_no_access_pvr_plural      = "The following files could not be accessed on the PVR:";
c_no_access_pc              = "The following file could not be accessed on the PC:";
c_no_access_pc_plural       = "The following files could not be accessed on the PC:";
c_folder_file_clash         = "The folder {0} could not be created because there exists a file of the same name.";
b_ok                        = "OK";
b_cancel                    = "Cancel";
b_yes                       = "Yes";
b_no                        = "No";
b_close                     = "Close";
b_cancel_transfer           = "Cancel transfer";
c_title1_copy               = "Copying File(s)";
c_title1_move               = "Moving File(s)";
c_title1b_copy              = "Copying File {0} of {1}";
c_title1b_move              = "Moving File {0} of {1}";
c_title2_to_pc              = "[PVR --> PC]";
c_title2_to_pvr             = "[PC --> PVR]";
c_finding                   = "Finding files...";
c_completion                = "On completion:";
c_sleep                     = "Sleep";
c_hibernate                 = "Hibernate";
c_shutdown                  = "Shutdown";
c_turbo_changing            = "Turbo mode [Changing...]";
o_exist                     = "Eine Datein mit diesem Namen existiert bereits.";
o_exist_plural              = "Dateien mit diesem Namen existieren bereits.";
o_delete_pvr                = "Kopie auf PVR entfernen";
o_delete_pvr_plural         = "Kopien auf PVR entfernen";
o_delete_pc                 = "Kopie auf PC entfernen";
o_delete_pc_plural          = "Kopien auf PC entfernen";
o_correct_plural            = "Files have correct size";
o_correct                   = "File has correct size";
o_undersized_plural         = "Die vorhandenen Dateien sind kleiner";
o_undersized                = "Die vorhandene Datei ist kleiner";
o_oversized_plural          = "Die vorhandenen Dateien sind gr��er!";
o_oversized                 = "DIe vorhandene Datei ist gr��er!";
o_skip                      = "�berspringen";
o_overwrite                 = "�berschreiben";
o_resume                    = "Resume (recommended)";
o_skip_r                    = "Skip (recommended)";
f_title                     = "Zu wenig Speicher";
f_warning                   = "Nicht genug freier Speicherplatz!";
f_required                  = "Notwendig:";
f_available                 = "M�glich:";
f_proceed                   = "Weiter ausf�hren?";
d_title                     = "Delete Confirmation";
d_folder                    = "[Folder -- Contents will be deleted!!!]";
d_delete                    = "Delete the following item?";
d_delete_plural             = "Delete the following items?";
d_error                     = "An error occurred while deleting.";
m_rename_error              = "An error occurred during rename.";
m_folder_error              = "Error creating new folder.";
m_paste_error               = "Cannot paste to this location, since it is inside a folder being moved.";
m_new_folder_name           = "New Folder";
cm_copy_pc                  = "Copy to PC";
cm_info                     = "Show program information";
cm_move_pc                  = "Move to PC";
cm_select_all               = "Select all";
cm_copy_pvr                 = "Copy to PVR";
cm_move_pvr                 = "Move to PVR";
cm_explorer                 = "Show in Explorer";
cm_rename                   = "Rename";
cm_firmware                 = "Install firmware to PVR";
s_title                     = "Antares Settings";
s_choose                    = "Choose Columns";
s_rescale                   = "Rescale column widths automatically";
s_idle                      = "During a transfer, prevent Windows from automatically going to sleep when idle";
s_language                  = "Language selection:";
p_wintitle                  = "Programminformation";
p_title                     = "Titel:";
p_proglen                   = "L�nge der Sendung:";
p_reclen                    = "L�nge der Aufnahme:";
}
};
}
