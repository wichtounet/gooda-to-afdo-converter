gooda-to-afdo-converter
=======================

Simple utility application to convert Gooda Spreadsheets to AFDO (Google Patch to add sampling profiling to GCC). 

Build
-----

To build the application, CMake 2.8 and GCC 4.7 are necessary. Boost >= 1.41 is also necessary. 

    git clone git://github.com/wichtounet/gooda-to-afdo-converter.git
    cd gooda-to-afdo-converter
    cmake .
    make
    
Usage
-----

Once built, the application can be used to perform several tasks. 

To profile your application with Gooda and generate AFDO data use: 

    ./bin/converter --afdo --gooda=gooda_director --profile your_application [your_options]
    
or

    ./bin/converter --afdo --gooda=gooda_director --lbr --profile your_application [your_options]
  
to use LBR. 

If the --afdo option is not set with the --profile option, it just runs the collection script and gooda on the given program. 

If GOODA_DIR environemnt variable is set to the Gooda directory, there is no need to use the --gooda option. 

To read existing spreadsheets and convert them to AFDO data: 

    ./bin/converter spreadsheets_directory
    
Again, use --lbr to activate LBR mode. 

AFDO data can also be dumped to the console: 

    ./bin/converter --dump spreadsheets_directory

Use --full-dump to have all the data printed oud. 