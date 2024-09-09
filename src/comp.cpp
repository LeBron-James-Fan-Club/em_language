#include "comp.h"
#include "misc.h"

void compiler::Compiler_ResetOffset() {
    this->localOffset = 0;
    // 4 because its like accessing a[0] instead of a[1]
    this->paramOffset = 4;
    this->paramRegCount = 0;
}

int compiler::Compiler_GetLocalOffset(enum ASTPRIM type) {
    debug("the local offset is %d", this->localOffset);
    this->localOffset += (PrimSize(type) > 4) ? PrimSize(type) : 4;
    debug("after the local offset is %d", this->localOffset);
    return this->localOffset;
}

int compiler::Compiler_GetParamOffset(enum ASTPRIM type) {
    debug("the param offset is %d", this->paramOffset);
    this->paramOffset += (PrimSize(type) > 4) ? PrimSize(type) : 4;
    debug("after the param offset is %d", this->paramOffset);
    return this->paramOffset;
}

compiler::compiler(char *outfile) {
    this->outfile = fopen(outfile, "w");

    if (this->outfile == nullptr) {
        fatala("OSError: Unable to open file %s", outfile);
    }
}

compiler::~compiler() {
    fclose(this->outfile);
}
