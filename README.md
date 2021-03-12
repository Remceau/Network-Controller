Network Controller
==================

A software solution to manage multiple servers in a cluster or network. This repository is a temprorary read-only repository while development of the solution is in its early stages. Documentation may be missing and stability is not guaranteed.



Build Instructions
------------------

For lack of a better soltuion, building is currently done through a bash script and thus only possible on linux systems. The script depends on the following tools to be installed and accessible from the default path: gcc (compilation of c units), python3 (utilty for parsing json files) and sed (replacing environment variabels). Verify that these tools are properly installed and up to date before trying to build any of the targets. To build a target, run the following command in the root directory of this repository.

```sh
./compile <target>
```

Some targets may depend on environment specific variables that should not or must not be shared throughout git repositories. These environment variables are stored in the environment.json file. The following json file can be seen as a skeleton for the environment file.

```json
{
    "ssh-authserver": {
        "database": {
            "host": "localhost",
            "username": "sshd",
            "password": "n6Tun6uye8Xq3Uq7",
            "database": "Network"
        }
    }
}
```


Build Targets
-------------

Target         | Packages       | Description
-------------- | -------------- | -----------
ssh-authserver | ssh-authserver | Authentication server for the ssh deamon to support a database backend in combination with ssh certificates.