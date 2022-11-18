#!/usr/bin/env python3

# I got this from https://github.com/chrysh/kas-scripts/blob/main/repoManifest2kasYml.py

# Usage: repoManifest2kasyaml.py imput.xml

import xml.dom.minidom
import yaml
import sys
import os

def parseManifestXML(xmlfile):
    doc = xml.dom.minidom.parse(xmlfile)

    projects = doc.getElementsByTagName("project")
    remotes  = doc.getElementsByTagName("remote")
    default = doc.getElementsByTagName("default")

    rems = {}
    for rem in remotes:
        rems[rem.getAttribute("name")] = rem.getAttribute("fetch")

    defnode = default.pop()
    defname = defnode.getAttribute("remote")
    defrem = rems[defname] if defname else ''
    defrev = defnode.getAttribute("revision")

    data = {}
    for proj in projects:
        [ path, name, remote, revision, upstream] = [
            proj.getAttribute("path"),
            proj.getAttribute("name"),
            proj.getAttribute("remote"),
            proj.getAttribute("revision"),
            proj.getAttribute("upstream") ]

        #print([name.rsplit('/', 1), path, name, remote, revision])
        if not remote:
            remote = defrem+"/"
        else:
            remote = rems[remote]+"/"
        if not revision:
            revision = defrev if defrev else upstream
        try:
            pname = name.rsplit('/', 1)[1]
        except IndexError:
            pname = name
        # FIXME
        if pname != "meta-openembedded":
            data[pname]= {"url":remote+name, "refspec":revision, "path":path}
        else:
            data[pname]= {"url":remote+name, "refspec":revision, "path":path, "layers":{"meta-filesystems":"", "meta-multimedia":"", "meta-networking":""}}
    return data


def saveHeader(outf):
    header = """header:\n    version: 3\n\n"""

    lfile="./auto.conf"
    if os.path.exists(lfile):
        print("Header file exists!")
        with open(lfile, 'r') as lconf:
            lines = lconf.readlines()
            for line in lines:
                if not (line.startswith('#')) and not line.isspace():
                    print(line)
                    if line.startswith("DISTRO"):
                        value = line.split()[2].replace("\"","")
                        header = "".join([header, "distro: ", value, "\n"])
                    if line.startswith("MACHINE"):
                        value = line.split()[2].replace("\"","")
                        header = "".join([header, "machine: ", value, "\n"])
    header = "".join([header, "target: core-image-minimal\n"])
    print(["Header: ", header])
    outf.write(header)

def saveFooter(outf):
    lfile="./local.conf"
    if os.path.exists(lfile):
        footer = '''\n\nlocal_conf_header:\n    meta-custom: | \n'''
        print("Footer file exists!")
        with open(lfile, 'r') as lconf:
            lines = lconf.readlines()
            for line in lines:
                if not (line.startswith('#')) and not line.isspace():
                    footer = "        ".join([footer, line])
        # print(["Footer: ", footer])
        outf.write(footer)

def saveToYAML(data, outfile):
    with open(outfile, 'w') as outf:
        saveHeader(outf)
        data = {"repos" : data}
        yaml.dump(data, outf, default_flow_style=False)

        saveFooter(outf)

if __name__ == "__main__":
    if (len(sys.argv) != 2):
        print("Usage: " + sys.argv[0] + " infile")
        sys.exit(0)

    infile = sys.argv[1]
    outfile = infile.replace("xml", "yml")
    if (infile == outfile):
        print("Input file " + infile + " is not a manifest xml file")
        sys.exit(0)
    print("Saving YAML to " + outfile)

    newsitems = parseManifestXML(infile)
    saveToYAML(newsitems, outfile)
