#include <qapplication.h>
#include <inputbox.h>
#include <qinputdialog.h>
int main(int argc, char* argv[]) 
{
    QApplication app(argc, argv);
    KBInputBox ib("title", "Viel test, text, text, text ""Viel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, text", 
		  AB_BANKING_INPUT_FLAGS_CONFIRM, 2, 40, 0, "InputBox", true);
    ib.exec();

    KBInputBox ib2("title", "Viel test, text, text, text ",//"Viel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, text", 
		  AB_BANKING_INPUT_FLAGS_CONFIRM, 2, 40, 0, "InputBox", true);
    ib2.exec();

    //bool ok;
    //QInputDialog::getText("title", "Viel test, text, text, text ""Viel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, textViel test, text, text, text", QLineEdit::Password, "", &ok);
    return 0;
}
