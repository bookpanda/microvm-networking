```bash
sudo apt update
sudo apt install acl

sudo setfacl -m u:${USER}:rw /dev/kvm

# need to setup terraform + ansible (install firecracker) for m6i.large?
terraform init
terraform plan
terraform apply
terraform destroy

chmod 400 firecracker-key.pem
ssh -i "firecracker-key.pem" ubuntu@ec2-13-212-127-192.ap-southeast-1.compute.amazonaws.com
```