namespace hs::types {


template<typename BaseTypes = StdBaseTypes>
double GetNumericValue(const StringId& variable) {
    if(!definitions->IsNumericVariable(variable)) {
        spdlog::error(
            "Variable {} is unknown or is not numeric",
            variable
            );
        return 0.0;
    }

}


}
