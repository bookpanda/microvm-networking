```bash
sudo apt update
sudo apt install acl

sudo setfacl -m u:${USER}:rw /dev/kvm

# need to setup terraform + ansible (install firecracker) for m6i.large?
terraform init
terraform plan
terraform apply
terraform destroy
```