#
# i.MX8 specific processing
#

# create uuu.auto file for image
addtask do_create_uuu_auto_script after do_image_complete before do_build

python do_create_uuu_auto_script() {
    def do_copyfile (src, destdir):
        import shutil

        bb.note("Copying %s to %s" % (src, destdir))
        shutil.copyfile(src, os.path.join(destdir, os.path.basename(src)))

    bb.note("DEPLOY_DIR_IMAGE='%s'" % d.expand("${DEPLOY_DIR_IMAGE}"))
    bb.note("BPN='%s'" % d.expand("${BPN}"))

    from string import Template

    machine = d.getVar('MACHINE')
    imxboot_target = d.getVar('IMXBOOT_TARGETS')
    image = d.getVar('BPN')

    uuu_template_path = os.path.join(d.getVar('DEPLOY_DIR_IMAGE'), "uuu.auto.template")
    img_dirname = os.path.join(d.getVar('DEPLOY_DIR_IMAGE'), d.getVar('BPN'))
    uuu_auto_path = os.path.join(img_dirname, "uuu.auto")

    bb.utils.mkdirhier(img_dirname)

    f = open(uuu_template_path, 'r')
    uuu_template_string = f.read()

    bb.note("Creating '%s' from '%s'" % (uuu_auto_path, uuu_template_path))
    try:
        with open(uuu_auto_path, 'w') as uuu_script:
            uuu_template = Template(uuu_template_string)
            uuu_auto = uuu_template.substitute(image=image, machine=machine, imxboot_target=imxboot_target, image_bootloader=d.getVar('IMAGE_BOOTLOADER'))
            uuu_script.write(uuu_auto)
    except OSError:
        bb.fatal('Unable to open %s' % (uuu_auto_path))

    do_copyfile("%s/%s-%s.bin-mfg-%s" % (d.getVar('DEPLOY_DIR_IMAGE'), \
        d.getVar('IMAGE_BOOTLOADER'), machine, imxboot_target), img_dirname)
    do_copyfile("%s/%s-%s.bin-default-%s" % (d.getVar('DEPLOY_DIR_IMAGE'), \
        d.getVar('IMAGE_BOOTLOADER'), machine, imxboot_target), img_dirname)
    do_copyfile("%s/karo-image-bootfs-%s.ext4" % \
        (d.getVar('DEPLOY_DIR_IMAGE'), machine), img_dirname)
    do_copyfile("%s/%s-%s.ext4" % \
        (d.getVar('DEPLOY_DIR_IMAGE'), image, machine), img_dirname)
}
