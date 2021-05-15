for dofile in tests.script/*; do
    
    if [ -d $dofile ]; then
        continue
    fi
    
    echo ./ref/fraig-mac -F ${dofile} &> ./output/"$(basename $dofile)"-ref;
    ./ref/fraig-mac -F ${dofile} &> ./output/"$(basename $dofile)"-ref;

done;
