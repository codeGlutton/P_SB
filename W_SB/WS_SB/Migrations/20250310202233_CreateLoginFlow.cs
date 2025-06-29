using System;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace WS_SB.Migrations
{
    /// <inheritdoc />
    public partial class CreateLoginFlow : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.CreateTable(
                name: "Registration",
                columns: table => new
                {
                    Id = table.Column<int>(type: "int", nullable: false)
                        .Annotation("SqlServer:Identity", "1, 1"),
                    Name = table.Column<string>(type: "nvarchar(40)", maxLength: 40, nullable: false),
                    Password = table.Column<string>(type: "nvarchar(40)", maxLength: 40, nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_Registration", x => x.Id);
                });

            migrationBuilder.CreateTable(
                name: "Account",
                columns: table => new
                {
                    Id = table.Column<int>(type: "int", nullable: false)
                        .Annotation("SqlServer:Identity", "1, 1"),
                    SnsType = table.Column<int>(type: "int", nullable: false),
                    SnsSub = table.Column<string>(type: "nvarchar(450)", nullable: false),
                    RegisterDBId = table.Column<int>(type: "int", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_Account", x => x.Id);
                    table.ForeignKey(
                        name: "FK_Account_Registration_RegisterDBId",
                        column: x => x.RegisterDBId,
                        principalTable: "Registration",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "Player",
                columns: table => new
                {
                    Id = table.Column<int>(type: "int", nullable: false)
                        .Annotation("SqlServer:Identity", "1, 1"),
                    Name = table.Column<string>(type: "nvarchar(40)", maxLength: 40, nullable: true),
                    CreatedTime = table.Column<DateTime>(type: "date", nullable: false),
                    CostumeSetting = table.Column<int>(type: "int", nullable: false),
                    Score = table.Column<int>(type: "int", nullable: false),
                    AccountDBId = table.Column<int>(type: "int", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_Player", x => x.Id);
                    table.ForeignKey(
                        name: "FK_Player_Account_AccountDBId",
                        column: x => x.AccountDBId,
                        principalTable: "Account",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateIndex(
                name: "IX_Account_RegisterDBId",
                table: "Account",
                column: "RegisterDBId",
                unique: true,
                filter: "[RegisterDBId] IS NOT NULL");

            migrationBuilder.CreateIndex(
                name: "IX_Account_SnsType_SnsSub",
                table: "Account",
                columns: new[] { "SnsType", "SnsSub" },
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_Player_AccountDBId",
                table: "Player",
                column: "AccountDBId");

            migrationBuilder.CreateIndex(
                name: "IX_Player_Score",
                table: "Player",
                column: "Score",
                filter: "[Score] > 500");

            migrationBuilder.CreateIndex(
                name: "IX_Registration_Name",
                table: "Registration",
                column: "Name",
                unique: true);
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropTable(
                name: "Player");

            migrationBuilder.DropTable(
                name: "Account");

            migrationBuilder.DropTable(
                name: "Registration");
        }
    }
}
