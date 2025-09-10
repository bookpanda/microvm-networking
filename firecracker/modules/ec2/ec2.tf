resource "aws_instance" "firecracker_ec2" {
  ami           = var.ami
  instance_type = var.instance_type

  security_groups = [aws_security_group.sg_firecracker.id]
  key_name        = aws_key_pair.generated_key_pair.key_name
  #   user_data = templatefile("${path.module}/install_wordpress.sh", {
  #     DB_NAME        = "${var.database_name}"
  #     DB_USER        = "${var.database_user}"
  #     DB_PASS        = "${var.database_pass}"
  #     DB_HOST        = "${aws_network_interface.eni_db_app_inet.private_ip}:3306"
  #     DB_PREFIX      = "wp_"
  #     WP_URL         = "http://${aws_eip.app_inet_eip.public_ip}"
  #     WP_ADMIN_USER  = "${var.admin_user}"
  #     WP_ADMIN_PASS  = "${var.admin_pass}"
  #     WP_ADMIN_EMAIL = "${var.admin_email}"
  #     WP_TITLE       = "Cloud"
  #     REGION         = "${var.region}"
  #     BUCKET_NAME    = "${var.bucket_name}"
  #   })

  tags = {
    Name = "firecracker-ec2"
  }
}
