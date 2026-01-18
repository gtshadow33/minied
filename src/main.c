#include "editor.h"
#include "terminal.h"
#include <stddef.h>

int main(int argc,char* argv[]){
    enable_raw_mode();
    Editor editor;
    const char* file=(argc>1)?argv[1]:NULL;
    editor_init(&editor,file);

    while(1){
        editor_refresh_screen(&editor);
        editor_process_key(&editor);
    }
}
