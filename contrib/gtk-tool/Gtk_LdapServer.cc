#include "Gtk_LdapServer.h"
#include <gtk--/base.h>

Gtk_LdapServer::Gtk_LdapServer() : Gtk_TreeItem() {
	this->hostname = NULL;
	this->par = NULL;
	this->base_dn = NULL;
	this->port = 0;
}

Gtk_LdapServer::Gtk_LdapServer(My_Window *w, char *h, int p) : Gtk_TreeItem() {
	char *s, *s2;
	this->par = w;
	this->hostname = h;
	this->port = p;
	this->notebook = NULL;
	debug("%s %i\n", this->hostname, this->port);
	this->setType(1);
	this->getConfig();
}

Gtk_LdapServer::Gtk_LdapServer(GtkTreeItem *t) : Gtk_TreeItem(t) {
}

Gtk_LdapServer::~Gtk_LdapServer() {
	debug("Bye\n");
	delete this;
}

void Gtk_LdapServer::setType(int t) {
	debug("Gtk_LdapServer::setType(%i)\n", t);
	Gtk_Pixmap *xpm_icon;
	Gtk_Label *label;
	char *c = NULL;
	if (this->getchild() != NULL) {
		xpm_label = new Gtk_HBox(GTK_HBOX(this->getchild()->gtkobj()));
		xpm_label->remove_c(xpm_label->children()->nth_data(0));
		xpm_label->remove_c(xpm_label->children()->nth_data(0));
	}
	else xpm_label = new Gtk_HBox();
	debug(this->hostname);
	if (strcasecmp(this->hostname,"localhost") == 0)
		xpm_icon=new Gtk_Pixmap(*xpm_label, local_server);
	else xpm_icon=new Gtk_Pixmap(*xpm_label, remote_server);
//	sprintf(c, "%s:%i", this->hostname, this->port);
//	printf("%s\n", c);
	label = new Gtk_Label(this->hostname);
	xpm_label->pack_start(*xpm_icon, false, false, 1);
	xpm_label->pack_start(*label, false, false, 1);
	if (this->getchild() == NULL) this->add(xpm_label);
	label->show();
	xpm_label->show();
	xpm_icon->show();
}

int Gtk_LdapServer::showDetails() {
	debug("Gtk_LdapServer::showDetails()\n");
	if (this->notebook != NULL) {
//		debug("Have notebook here");
		if (par->viewport->getchild() != NULL) {
//			debug(" and viewport has children");
			par->viewport->remove_c(par->viewport->getchild()->gtkobj());
//			debug(" which have been removed");
		}
//		else debug(" and viewport without children");
		par->viewport->add(this->notebook);
		this->notebook->show();
		par->viewport->show();
		return 0;
	}
	if (this->getOptions() != 0) return 1;
	this->showDetails();
//	debug("done\n");
	return 0;
}

int Gtk_LdapServer::getConfig() {
	debug("Gtk_LdapServer::getConfig()\n");
	int error, entriesCount;
	LDAPMessage *entry, *result_identifier;
	BerElement *ber;
	char *attribute, **t;

	if ((this->ld = ldap_open(this->hostname, this->port)) == NULL) {
		perror("connection");
	}

	error = ldap_search_s(this->ld, "cn=config", LDAP_SCOPE_BASE, "objectclass=*", NULL, 0, &result_identifier);	
	entriesCount = ldap_count_entries(this->ld, result_identifier);
	if (entriesCount == 0) {
		return 0;
	}

	debug("%i entry\n", entriesCount);
	for (entry = ldap_first_entry(this->ld, result_identifier); entry != NULL; entry = ldap_next_entry(this->ld, result_identifier)) {
		for (attribute = ldap_first_attribute(this->ld, entry, &ber); attribute != NULL; attribute = ldap_next_attribute(this->ld, entry, ber)) {
			debug("Attrib: %s\n", attribute);
			if (strcasecmp(attribute, "database") == 0) {
				debug("have database here\n");
				this->databases = new G_List<char>;
				t = ldap_get_values(this->ld, entry, attribute);
				for (int i=0; i<ldap_count_values(t); i++) {
					this->databases->append(strdup(t[i]));
				}
				ldap_value_free(t);
				debug("databases loaded\n");
				for (int i=0; i<this->databases->length(); i++) {
					debug("database(%i) %s\n", i, this->databases->nth_data(i));
				}	
			}
		}
		debug("entry done\n");
	}
//	debug("got %i entries\n", entriesCount);
	return entriesCount;
}

char* Gtk_LdapServer::getOptDescription(int option) {
	debug("Gtk_LdapServer::getOptDescription(%i) ", option);
	char *c;
	switch (option) {
		case LDAP_OPT_API_INFO: c = "API info"; break;
		case LDAP_OPT_CLIENT_CONTROLS: c = "Client controls"; break;
		case LDAP_OPT_DEREF: c = "Dereference"; break;
		case LDAP_OPT_DESC: c = "Description"; break;
		case LDAP_OPT_DNS: c = "DNS Lookup"; break;
		case LDAP_OPT_ERROR_NUMBER: c = "Error number"; break;
		case LDAP_OPT_ERROR_STRING: c = "Error string"; break;
		case LDAP_OPT_SIZELIMIT: c = "Size limit"; break;
		case LDAP_OPT_TIMELIMIT: c = "Time limit"; break;
		case LDAP_OPT_REFERRALS: c = "Referrals"; break;
		case LDAP_OPT_RESTART: c = "Started"; break;
		case LDAP_OPT_PROTOCOL_VERSION: c = "Protocol version"; break;
		case LDAP_OPT_HOST_NAME: c = "Host name"; break;
		case LDAP_OPT_SERVER_CONTROLS: c = "Server controls"; break;
		default: c = "No description"; break;
	}
	debug("%s\n", c);
	return c;
}

int Gtk_LdapServer::getOptType(int option) {
	debug("Gtk_LdapServer::getOptType(%i) ", option);
	int type; /* 0 = int, 1 = string, 2 = boolean */
	switch(option) {
		/* ints */
		case LDAP_OPT_DEREF:
		case LDAP_OPT_DESC:
		case LDAP_OPT_SIZELIMIT:	
		case LDAP_OPT_TIMELIMIT:
		case LDAP_OPT_ERROR_NUMBER:
		case LDAP_OPT_PROTOCOL_VERSION: type = 0; break;
		/* strings */
		case LDAP_OPT_ERROR_STRING:
		case LDAP_OPT_HOST_NAME: type = 1; break;
		/* bools */
		case LDAP_OPT_REFERRALS:
		case LDAP_OPT_DNS:
		case LDAP_OPT_RESTART: type = 2; break;
		case LDAP_OPT_SERVER_CONTROLS:
		case LDAP_OPT_CLIENT_CONTROLS:
		case LDAP_OPT_API_INFO:
		default: type = 0; break;
	}
	debug("%i\n", type);
	return type;
}

int Gtk_LdapServer::getOptions() {
	debug("Gtk_LdapServer::getOptions()\n");
	if (this->notebook != NULL) return 0;
	Gtk_HBox *hbox, *mini_hbox;
	Gtk_VBox *vbox, *mini_vbox;
	Gtk_Table *table;
	Gtk_Label *label;	
	Gtk_RadioButton *radio1, *radio2;
	char *s_value;
	int i_value;
	char *thing;
	int things[9] = {
		LDAP_OPT_API_INFO,
	//	LDAP_OPT_CLIENT_CONTROLS,
	//	LDAP_OPT_DESC,
		LDAP_OPT_DEREF,
		LDAP_OPT_DNS,
	//	LDAP_OPT_ERROR_NUMBER,
	//	LDAP_OPT_ERROR_STRING,
		LDAP_OPT_HOST_NAME,
		LDAP_OPT_PROTOCOL_VERSION,
		LDAP_OPT_REFERRALS,
		LDAP_OPT_RESTART,
	//	LDAP_OPT_SERVER_CONTROLS,
		LDAP_OPT_SIZELIMIT,
		LDAP_OPT_TIMELIMIT
	};

/*	if (GTK_TREE_ITEM(this->gtkobj())->subtree == NULL) {
		this->getSubtree();
	} */

//	debug("getting ldap options");
//	vbox = new Gtk_VBox();
	table = new Gtk_Table(11, 2, TRUE);

	for (int i=0; i<9; i++) {
	//	debug("%i\n", i);
		hbox = new Gtk_HBox(TRUE, 2);
		hbox->border_width(2);
		thing = this->getOptDescription(things[i]);
		label = new Gtk_Label(thing);
		label->set_justify(GTK_JUSTIFY_LEFT);
		label->set_alignment(0, 0);
		hbox->pack_start(*label);
		label->show();
		int tipus = this->getOptType(things[i]);
		switch (tipus) {
			case 0:
				ldap_get_option(NULL, things[i], &i_value);
				debug("%s value %d\n", thing, i_value);
				sprintf(s_value, "%d", i_value);
				label = new Gtk_Label(s_value);
				label->set_justify(GTK_JUSTIFY_LEFT);
				label->set_alignment(0, 0);
				hbox->pack_end(*label);
				label->show();
				break;
			case 1:
				ldap_get_option(this->ld, things[i], &s_value);
				label = new Gtk_Label(s_value);
				label->set_justify(GTK_JUSTIFY_LEFT);
				label->set_alignment(0, 0);
				hbox->pack_end(*label);
				label->show();
				break;
			case 2:
				ldap_get_option(this->ld, things[i], &i_value);
			//	sprintf(s_value, "%s", i_value == (int) LDAP_OPT_ON ? "on" : "off");
			//	label = new Gtk_Label(s_value);
				radio1 = new Gtk_RadioButton(static_cast<GSList*>(0), "Enabled");
				radio2 = new Gtk_RadioButton(*radio1, "Disabled");
				if (i_value == 1) radio1->set_state(true);
				else radio2->set_state(true);
				mini_hbox = new Gtk_HBox(FALSE, 2);
				mini_hbox->border_width(2);
				mini_hbox->pack_start(*radio1);
				radio1->show();
				mini_hbox->pack_end(*radio2);
				radio2->show();
				hbox->pack_end(*mini_hbox);
				mini_hbox->show();
				break;
			default:
				label = new Gtk_Label("Nothing");
				label->set_justify(GTK_JUSTIFY_LEFT);
				label->set_alignment(0, 0);
				hbox->pack_end(*label);
				label->show();
				break;
		}
	//	hbox->pack_end(*label);
	//	label->show();
		table->attach_defaults(*hbox, 0, 2, i, i+1);
		hbox->show();
	}
	table->border_width(2);
	this->notebook = new Gtk_Frame("LDAP Options");
	this->notebook->add(*table);
	table->show();
	return 0;
}

Gtk_Tree* Gtk_LdapServer::getSubtree() {
	debug("Gtk_LdapServer::getSubtree()\n");
	Gtk_LdapTree *tree, *subtree;
	Gtk_LdapTreeItem *treeitem;
	int entries;

	debug("this->hostname=%s\n", this->hostname);
	debug("this->port=%i", this->port);

	char *c;
	char *tok;

	int len = this->databases->length();
	debug("this->databases->length()=%i\n", len);

	tree = new Gtk_LdapTree();
	for (int i=0; i<len; i++) {
		tok = strdup(this->databases->nth_data(i));
		tok = strtok(tok, ":");
	//	c = strtok(NULL, " ");
		c = strtok(NULL, "\0");
		debug("database %i %s\n", i, c);
		treeitem = new Gtk_LdapTreeItem(c, this->par, this->ld);
		subtree = treeitem->getSubtree(this->ld, 1);
		debug("inserting %s into %s\n", treeitem->rdn, this->hostname);
		tree->append(*treeitem);
		treeitem->set_subtree(*subtree);
		treeitem->show();
	//	tree->show();
	}
//	this->set_subtree(*tree);
	debug("getTree() done\n");
	return tree;
}

void Gtk_LdapServer::show_impl() {
	debug("%s showed\n", this->hostname);
	Gtk_c_signals_Item *sig=(Gtk_c_signals_Item *)internal_getsignalbase();
	sig->show(GTK_WIDGET(gtkobj()));
}

void Gtk_LdapServer::select_impl() {
	debug("%s selected\n", this->hostname);
	Gtk_c_signals_Item *sig=(Gtk_c_signals_Item *)internal_getsignalbase();
	if (this->showDetails() == 0) debug("%s select_impl done\n", this->hostname);
	if (!sig->select) return;
	sig->select(GTK_ITEM(gtkobj()));
}

void Gtk_LdapServer::collapse_impl() {
	debug("%s collapsed\n", this->hostname);
	Gtk_c_signals_TreeItem *sig=(Gtk_c_signals_TreeItem *)internal_getsignalbase();
	if (!sig->collapse) return;
	sig->collapse(GTK_TREE_ITEM(gtkobj()));
//	gtk_widget_hide(GTK_WIDGET(GTK_TREE(GTK_TREE_ITEM (this->gtkobj())->subtree)));
}

void Gtk_LdapServer::expand_impl() {
	debug("%s expanded\n", this->hostname);
	Gtk_c_signals_TreeItem *sig=(Gtk_c_signals_TreeItem *)internal_getsignalbase();
	if (!sig->expand) return;
	sig->expand(GTK_TREE_ITEM(gtkobj()));
//	Gtk_Tree *t;
//	t = new Gtk_Tree(GTK_TREE(GTK_TREE_ITEM(this->gtkobj())->subtree));
//	bool vis = t->visible();
//	if (vis == false) {
//		gtk_widget_show(GTK_WIDGET(GTK_TREE(GTK_TREE_ITEM (this->gtkobj())->subtree)));
//		cout << this->dn << " expanded" << endl;
//	}
//	else {
//		gtk_widget_hide(GTK_WIDGET(GTK_TREE(GTK_TREE_ITEM (this->gtkobj())->subtree)));
//		cout << this->dn << " collapsed" << endl;
//	}
}
