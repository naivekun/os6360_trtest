CLI_type_dict["vrf_id"] = {
    checker = CLI_check_param_number,
    min=0,
    max=4095,
    help="a Virtual Router Id (0-4095)"
}

CLI_type_dict["vrf_id_without_0"] = {
    checker = CLI_check_param_number,
    min=1,
    max=4095,
    help="a Virtual Router Id (1-4095)"
}
