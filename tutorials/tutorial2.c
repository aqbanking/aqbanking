/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


/***************************************************************************
 * This tutorial shows the list of accounts currently known to AqBanking.  *
 *                                                                         *
 * It also gives an introduction into the usage of XXX_List2's and list2   *
 * iterators.                                                              *
 *                                                                         *
 * You must either choose a GUI implementation to be used with AqBanking   *
 * or create one yourself by implementing the user interface callbacks of  *
 * LibGwenhywfar.                                                          *
 *                                                                         *
 * However, for simplicity reasons we use the console GUI implementation   *
 * which implements these callbacks for you.                               *
 *                                                                         *
 * There are other GUI implementations, e.g. for GTK2, QT3, QT4 and FOX16. *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <aqbanking/banking.h>
#include <gwenhywfar/cgui.h>



int main(int argc, char **argv) {
  AB_BANKING *ab;
  AB_ACCOUNT_LIST2 *accs;
  int rv;
  GWEN_GUI *gui;

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  ab=AB_Banking_new("tutorial2", 0, 0);
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Error on init (%d)\n", rv);
    return 2;
  }

  rv=AB_Banking_OnlineInit(ab);
  if (rv) {
    fprintf(stderr, "Error on onlineinit (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "AqBanking successfully initialized.\n");

  /* Get a list of accounts which are known to AqBanking.
   * There are some pecularities about the list returned:
   * The list itself is owned by the caller (who must call
   * AB_Account_List2_free() as we do below), but the elements of that
   * list (->the accounts) are still owned by AqBanking.
   * Therefore you MUST NOT free any of the accounts within the list returned.
   * This also rules out calling AB_Account_List2_freeAll() which not only
   * frees the list itself but also frees all its elements.
   *
   * The rest of this tutorial shows how lists are generally used by
   * AqBanking.
   */
  accs=AB_Banking_GetAccounts(ab);
  if (accs) {
    AB_ACCOUNT_LIST2_ITERATOR *it;

    /* List2's are traversed using iterators. An iterator is an object
     * which points to a single element of a list.
     * If the list is empty NULL is returned.
     */
    it=AB_Account_List2_First(accs);
    if (it) {
      AB_ACCOUNT *a;

      /* this function returns a pointer to the element of the list to
       * which the iterator currently points to */
      a=AB_Account_List2Iterator_Data(it);
      while(a) {
	AB_PROVIDER *pro;

	/* every account is assigned to a backend (sometimes called provider)
	 * which actually performs online banking tasks. We get a pointer
	 * to that provider/backend with this call to show its name in our
         * example.*/
	pro=AB_Account_GetProvider(a);
	fprintf(stderr,
		"Account: %s (%s) %s (%s) [%s]\n",
		AB_Account_GetBankCode(a),
		AB_Account_GetBankName(a),
		AB_Account_GetAccountNumber(a),
		AB_Account_GetAccountName(a),
                /* the name of the provider/backend as decribed above */
		AB_Provider_GetName(pro));
	/* this function lets the iterator advance to the next element in
	 * the list, so a following call to AB_Account_List2Iterator_Data()
	 * would return a pointer to the next element.
	 * This function also returns a pointer to the next element of the
	 * list. If there is no next element then NULL is returned. */
	a=AB_Account_List2Iterator_Next(it);
      }
      /* the iterator must be freed after using it */
      AB_Account_List2Iterator_free(it);
    }
    /* as discussed the list itself is only a container which has to be freed
     * after use. This explicitly does not free any of the elements in that
     * list, and it shouldn't because AqBanking still is the owner of the
     * accounts */
    AB_Account_List2_free(accs);
  }


  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on online deinit (%d)\n", rv);
    return 3;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 3;
  }
  AB_Banking_free(ab);
  return 0;
}


