output "firecracker_ip" {
  value = aws_instance.firecracker_ec2.public_ip
}
