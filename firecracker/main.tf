module "ec2" {
  source   = "./modules/ec2"
  ami      = var.ami
  key_name = var.key_name
}
