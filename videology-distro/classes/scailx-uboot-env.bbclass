## Export uboot-environment from the SCAILX_UBOOT_ENV set in CONF files

#############################################################################
## File building functions
#############################################################################
def output_env_file(d, file):
    with open(file, 'w+') as f:
        env = d.getVarFlags('SCAILX_UBOOT_ENV')
        for k,v in env.items():
            line = ' '.join(v.split())
            f.write("%s=%s"%(k, line) + os.linesep)

#############################################################################
