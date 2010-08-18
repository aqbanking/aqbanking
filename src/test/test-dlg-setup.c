#include <gwen-gui-gtk2/gtk2_gui.h>
#include <aqbanking/banking.h>
#include <aqbanking/dlg_setup.h>
#include <assert.h>

int main(int argc, char **argv) {
  GWEN_GUI *gui;
  int r;

  gtk_init(&argc, &argv);

  gui = Gtk2_Gui_new();
  GWEN_Gui_SetGui(gui);

  {
    AB_BANKING *ab;
    ab = AB_Banking_new("test-dlg-setup", NULL, 0);

    r = AB_Banking_Init(ab);
    assert(r == 0);

    r = AB_Banking_OnlineInit(ab);
    assert(r == 0);

    {
      GWEN_DIALOG *dlg;
      dlg = AB_SetupDialog_new(ab);

      r = GWEN_Gui_ExecDialog(dlg, 0);
      if (r <= 0)
        {
          printf("Dialog was aborted/rejected\n");
        }
      else
        {
          printf("Dialog accepted, all fine\n");
        }
      GWEN_Dialog_free(dlg);
    }

    r = AB_Banking_OnlineFini(ab);
    assert(r == 0);

    r = AB_Banking_Fini(ab);
    assert(r == 0);
    AB_Banking_free(ab);
  }

  GWEN_Gui_free(gui);
  return 0;
}
