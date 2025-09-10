module "ec2" {
  source        = "./modules/ec2"
  instance_type = var.instance_type
  ami           = var.ami
  key_name      = var.key_name
}
