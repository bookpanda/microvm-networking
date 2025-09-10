resource "aws_security_group" "sg_firecracker" {
  name        = "sg_firecracker"
  description = "Security group for firecracker EC2 instance"
  vpc_id      = data.aws_vpc.default.id

  ingress {
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  ingress {
    from_port   = 80
    to_port     = 80
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  ingress {
    from_port   = 443
    to_port     = 443
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1" # all outbound traffic
    cidr_blocks = ["0.0.0.0/0"]
  }
}

# resource "aws_network_interface" "eni_firecracker" {
#   subnet_id       = var.subnet_id
#   private_ips     = ["10.0.1.100"] # static private IP, must be within the app_inet subnet CIDR range
#   security_groups = [aws_security_group.sg_firecracker.id]
#   tags = {
#     Name = "${var.vpc_name}-eni_app_inet"
#   }
# }
