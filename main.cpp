#include <cstring>

#include "cmdline.h"

int build(ParametersBuild& params);
int filter(ParametersFilter& params);

int main( int argc, char **argv) {
    bindopt::Options options;
    //TODO implement --log --ram --mask --compression
    if(argc > 1 and strcmp(argv[1], "build")==0) {
        ParametersBuild params(options);
        bindopt::bindopt(argc-2, argv+2, options);
        return build(params);
    } else if(argc > 1 and strcmp(argv[1], "filter")==0) {
        ParametersFilter params(options);
        bindopt::bindopt(argc-2, argv+2, options);
        return filter(params);
    } else {
        print_help();
    }
    return 0;
}
