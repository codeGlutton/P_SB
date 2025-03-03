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
                name: "User",
                columns: table => new
                {
                    Id = table.Column<int>(type: "int", nullable: false)
                        .Annotation("SqlServer:Identity", "1, 1"),
                    UserName = table.Column<string>(type: "nvarchar(40)", maxLength: 40, nullable: false),
                    CostumeSetting = table.Column<int>(type: "int", nullable: false),
                    AccountDBId = table.Column<int>(type: "int", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_User", x => x.Id);
                    table.ForeignKey(
                        name: "FK_User_Account_AccountDBId",
                        column: x => x.AccountDBId,
                        principalTable: "Account",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateTable(
                name: "Ranking",
                columns: table => new
                {
                    Id = table.Column<int>(type: "int", nullable: false)
                        .Annotation("SqlServer:Identity", "1, 1"),
                    Score = table.Column<int>(type: "int", nullable: false),
                    UserDBId = table.Column<int>(type: "int", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_Ranking", x => x.Id);
                    table.ForeignKey(
                        name: "FK_Ranking_User_UserDBId",
                        column: x => x.UserDBId,
                        principalTable: "User",
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
                name: "IX_Ranking_Score",
                table: "Ranking",
                column: "Score");

            migrationBuilder.CreateIndex(
                name: "IX_Ranking_UserDBId",
                table: "Ranking",
                column: "UserDBId",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_Registration_Name",
                table: "Registration",
                column: "Name",
                unique: true);

            migrationBuilder.CreateIndex(
                name: "IX_User_AccountDBId",
                table: "User",
                column: "AccountDBId");
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropTable(
                name: "Ranking");

            migrationBuilder.DropTable(
                name: "User");

            migrationBuilder.DropTable(
                name: "Account");

            migrationBuilder.DropTable(
                name: "Registration");
        }
    }
}
