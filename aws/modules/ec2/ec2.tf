data "aws_vpc" "default" {
  default = true
}

data "aws_subnets" "default" {
  filter {
    name   = "vpc-id"
    values = [data.aws_vpc.default.id]
  }
}

resource "aws_instance" "firecracker_ec2" {
  ami           = var.ami
  instance_type = var.instance_type
  subnet_id     = data.aws_subnets.default.ids[0]

  security_groups = [aws_security_group.sg_firecracker.id]
  key_name        = aws_key_pair.generated_key_pair.key_name
  user_data       = templatefile("${path.module}/user_data.sh", {})

  tags = {
    Name = "firecracker-ec2"
  }
}
