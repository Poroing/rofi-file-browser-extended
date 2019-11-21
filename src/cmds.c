#include <gmodule.h>

#include "defaults.h"
#include "types.h"
#include "cmds.h"
#include "util.h"


/**
 * Add an array of cmds to the ones currently stored in the private data.
 */
static void add_cmds ( FBCmd *cmds, int num_cmds, FileBrowserModePrivateData *pd );

/**
 * Compares the command strings of two cmds in lexicographic order.
 */
static gint compare_cmds ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data );

// ================================================================================================================= //

static void add_cmds ( FBCmd *cmds, int num_cmds, FileBrowserModePrivateData *pd )
{
    pd->cmds = g_realloc ( pd->cmds, ( pd->num_cmds + num_cmds ) * sizeof ( FBCmd ) );
    memcpy ( &pd->cmds[pd->num_cmds], cmds, num_cmds * sizeof ( FBCmd ) );
    pd->num_cmds += num_cmds;
    pd->show_cmds = pd->num_cmds > 0;
}

void set_user_cmds ( char **cmd_strs, FileBrowserModePrivateData *pd )
{
    if ( cmd_strs == NULL ) {
        return;
    }

    static int icon_sep_len = strlen ( OPEN_CUSTOM_CMD_ICON_SEP );
    static int name_sep_len = strlen ( OPEN_CUSTOM_CMD_NAME_SEP );

    int num_cmds = count_strv ( ( const char ** ) cmd_strs );
    FBCmd *cmds = g_malloc ( num_cmds * sizeof ( FBCmd ) );

    for ( int i = 0; i < num_cmds; i++ ) {
        char* cmd = cmd_strs[i];
        char* icon_name = g_strrstr ( cmd, OPEN_CUSTOM_CMD_ICON_SEP );
        char* name = g_strrstr ( cmd, OPEN_CUSTOM_CMD_NAME_SEP );

        if ( icon_name != NULL ) {
            *icon_name = '\0';
        }
        if ( name != NULL ) {
            *name = '\0';
        }

        FBCmd *fbcmd = &cmds[i];
        fbcmd->cmd = g_strdup ( cmd );
        fbcmd->icon_name = icon_name == NULL ? NULL : g_strdup ( &icon_name[icon_sep_len] );
        fbcmd->name = name == NULL ? NULL : g_strdup ( &name[name_sep_len] );
        fbcmd->icon = NULL;
    }

    add_cmds(cmds, num_cmds, pd);
    g_free ( cmds );
}

void search_path_for_cmds ( FileBrowserModePrivateData *pd )
{
    char *path = g_strdup ( g_getenv ( "PATH" ) );
    if ( path == NULL ) {
        print_err ( "Could not get $PATH environment variable to search for executables.\n" );
        return;
    }

    GHashTable *table = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, NULL );

    /* Search all directories in $PATH for files and insert file names into the hash table. */
    char* dirname = strtok ( path, G_SEARCHPATH_SEPARATOR_S );
    while ( dirname != NULL ) {
        GDir *dir = g_dir_open ( dirname, 0, NULL );

        if ( dir == NULL ) {
            print_err ( "Could not open directory \"%s\" in $PATH to search for executables.\n", dirname );

        } else {
            const char *filename;

            while ( ( filename = g_dir_read_name ( dir ) ) ) {
                char c0 = filename[0];
                if ( ( c0 >= '0' && c0 <= '9' ) || ( c0 >= 'a' && c0 <= 'z' ) || ( c0 >= 'A' && c0 <= 'Z' ) ) {
                    g_hash_table_insert ( table, g_strdup ( filename ), NULL );
                }
            }

            g_dir_close ( dir );
        }

        dirname = strtok ( NULL, G_SEARCHPATH_SEPARATOR_S );
    }

    g_free ( path );

    FBCmd *cmds = malloc ( g_hash_table_size ( table ) * sizeof ( FBCmd ) );
    int num_cmds = 0;

    for ( GList *list = g_hash_table_get_keys( table ); list != NULL; list = list->next ) {
        char *cmdstr = list->data;

        FBCmd *fbcmd = &cmds[num_cmds];
        fbcmd->cmd = cmdstr;
        fbcmd->name = NULL;
        fbcmd->icon_name = NULL;
        fbcmd->icon = NULL;

        num_cmds++;
    }

    g_hash_table_steal_all ( table );
    g_hash_table_destroy ( table );

    g_qsort_with_data ( cmds, num_cmds, sizeof ( FBCmd ), compare_cmds, NULL );

    add_cmds(cmds, num_cmds, pd);

    g_free ( cmds );
}

void destroy_cmds ( FileBrowserModePrivateData *pd )
{
    for ( int i = 0; i < pd->num_cmds; i++ ) {
        g_free( pd->cmds[i].cmd );
        g_free( pd->cmds[i].icon_name );
        g_free( pd->cmds[i].name );
    }
    g_free ( pd->cmds );
    pd->cmds = NULL;
    pd->num_cmds = 0;
    pd->show_cmds = false;
}

static gint compare_cmds ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data )
{
    const FBCmd *ca = a;
    const FBCmd *cb = b;
    return g_strcmp0 ( ca->cmd, cb->cmd );
}
