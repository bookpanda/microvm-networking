variable "region" {
  description = "The region in which the VPC will be created"
  type        = string
}

variable "availability_zone" {
  description = "The availability zone in which the VPC will be created"
  type        = string
}

variable "ami" {
  description = "The AMI for the EC2 instance"
  type        = string
}

variable "key_name" {
  description = "The key pair name for the EC2 instance"
  type        = string
  default     = "firecracker"
}
